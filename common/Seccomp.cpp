/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Code to lock-down the environment of the processes we run, to avoid
 * exotic or un-necessary system calls to be used to break containment.
 */

#include <config.h>

#include "Seccomp.hpp"

#include <common/Log.hpp>
#include <common/NumUtil.hpp>
#include <common/SigUtil.hpp>

#include <csignal>
#include <dlfcn.h>
#include <ftw.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sysexits.h>
#include <unistd.h>
#include <utime.h>

#ifdef __linux__
#include <linux/audit.h>
#include <linux/filter.h>
#if DISABLE_SECCOMP == 0
#include <linux/seccomp.h>
#endif
#include <signal.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#endif // __linux__

#if DISABLE_SECCOMP == 0
#ifndef SYS_SECCOMP
#  define SYS_SECCOMP 1
#endif

#if defined(__x86_64__)
#  define AUDIT_ARCH_NR AUDIT_ARCH_X86_64
#  define SECCOMP_REG(_ctx, _reg) ((_ctx)->uc_mcontext.gregs[(_reg)])
#  define SECCOMP_SYSCALL(_ctx)   SECCOMP_REG(_ctx, REG_RAX)
#elif defined(__aarch64__)
#  define AUDIT_ARCH_NR AUDIT_ARCH_AARCH64
#  define SECCOMP_REG(_ctx, _reg) ((_ctx)->uc_mcontext.regs[_reg])
#  define SECCOMP_SYSCALL(_ctx)   SECCOMP_REG(_ctx, 8)
#elif defined(__arm__)
#  define AUDIT_ARCH_NR AUDIT_ARCH_ARM
#  define SECCOMP_REG(_ctx, _reg) ((_ctx)->uc_mcontext.arm_##_reg)
#  define SECCOMP_SYSCALL(_ctx)   SECCOMP_REG(_ctx, r7)
#elif defined(__powerpc64__)
#  if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#    define AUDIT_ARCH_NR AUDIT_ARCH_PPC64LE
#  else
#    define AUDIT_ARCH_NR AUDIT_ARCH_PPC64
#  endif
#  define SECCOMP_REG(_ctx, _reg) ((_ctx)->uc_mcontext.regs->gpr[_reg])
#  define SECCOMP_SYSCALL(_ctx)   SECCOMP_REG(_ctx, 0)
#else
#  error "Platform does not support seccomp filtering yet - unsafe."
#endif

extern "C" {

static void handleSysSignal(int /* signal */,
                            siginfo_t *info,
                            void *context)
{
    ucontext_t* uctx = static_cast<ucontext_t*>(context);

    SigUtil::signalLogOpen();
    SigUtil::signalLogPrefix();
    SigUtil::signalLog("SIGSYS trapped with code: ");
    SigUtil::signalLogNumber(static_cast<std::size_t>(info->si_code));
    SigUtil::signalLog(" and context ");
    SigUtil::signalLogNumber(reinterpret_cast<std::size_t>(context));
    SigUtil::signalLog("\n");

    if (info->si_code != SYS_SECCOMP || !uctx)
        return;

    unsigned int syscall = SECCOMP_SYSCALL (uctx);

    SigUtil::signalLogPrefix();
    SigUtil::signalLog(" seccomp trapped signal, un-authorized sys-call: ");
    SigUtil::signalLogNumber(static_cast<std::size_t>(syscall));
    SigUtil::signalLog("\n");

    SigUtil::dumpBacktrace();
    SigUtil::signalLogClose();

    Util::forcedExit(EX_SOFTWARE);
}

} // extern "C"
#endif

namespace Seccomp {

bool lockdown([[maybe_unused]] Type type)
{
#if DISABLE_SECCOMP == 0
    #define ACCEPT_SYSCALL(name) \
        BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, __NR_##name, 0, 1), \
        BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_ALLOW)

    #define REJECT_SYSCALL(name, err) \
        BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, __NR_##name, 0, 1), \
        BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_ERRNO | (err & SECCOMP_RET_DATA))

    #define KILL_SYSCALL_FULL(fullname) \
        BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, fullname, 0, 1), \
        BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_TRAP)

    #define KILL_SYSCALL(name) \
        KILL_SYSCALL_FULL(__NR_##name)

    struct sock_filter filterCode[] = {
        // Check our architecture is correct.
        BPF_STMT(BPF_LD+BPF_W+BPF_ABS,  offsetof(struct seccomp_data, arch)),
        BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, AUDIT_ARCH_NR, 1, 0),
        BPF_STMT(BPF_RET+BPF_K,         SECCOMP_RET_KILL),

        // Load syscall number
        BPF_STMT(BPF_LD+BPF_W+BPF_ABS,  offsetof(struct seccomp_data, nr)),

        // First white-list the syscalls we frequently use.
        ACCEPT_SYSCALL(recvfrom),
        ACCEPT_SYSCALL(write),
        ACCEPT_SYSCALL(futex),

        // 'poll' implementation may use these epoll syscalls:
#if !defined(__NR_epoll_wait) && defined(__NR_epoll_pwait)
        ACCEPT_SYSCALL(epoll_pwait),
#else
        ACCEPT_SYSCALL(epoll_wait),
#endif
        ACCEPT_SYSCALL(epoll_ctl),
#if !defined(__NR_epoll_create) && defined(__NR_epoll_create1)
        ACCEPT_SYSCALL(epoll_create1),
#else
        ACCEPT_SYSCALL(epoll_create),
#endif
        ACCEPT_SYSCALL(close),
        ACCEPT_SYSCALL(nanosleep),

        // Now block everything that we don't like the look of.

        // FIXME: should we bother blocking calls that have early
        // permission checks we don't meet ?

#if 0
        // cf. eg. /usr/include/asm/unistd_64.h ...
        KILL_SYSCALL(ioctl),
        KILL_SYSCALL(mincore),
        KILL_SYSCALL(shmget),
        KILL_SYSCALL(shmat),
        KILL_SYSCALL(shmctl),
#endif
        REJECT_SYSCALL(execve, EPERM),
#ifdef __NR_execveat
        REJECT_SYSCALL(execveat, EPERM),
#endif
        KILL_SYSCALL(getitimer),
        KILL_SYSCALL(setitimer),
        KILL_SYSCALL(sendfile),
        KILL_SYSCALL(listen),  // server sockets
        KILL_SYSCALL(accept),  // server sockets
#if 0
        KILL_SYSCALL(wait4),
#endif
        KILL_SYSCALL(kill),   // !
#ifdef __NR_shmctl
        KILL_SYSCALL(shmctl),
#endif
        KILL_SYSCALL(ptrace), // tracing
        KILL_SYSCALL(capset),
#ifdef __NR_uselib
        KILL_SYSCALL(uselib),
#endif
        KILL_SYSCALL(personality), // !
        KILL_SYSCALL(vhangup),
#ifdef __NR_modify_ldt
        KILL_SYSCALL(modify_ldt), // !
#endif
#ifdef __PNR_modify_ldt
        KILL_SYSCALL_FULL(__PNR_modify_ldt), // !
#endif
        KILL_SYSCALL(pivot_root), // !
        KILL_SYSCALL(chroot),
        KILL_SYSCALL(acct),   // !
        KILL_SYSCALL(sync),   // I/O perf.
        KILL_SYSCALL(mount),
        KILL_SYSCALL(umount2),
        KILL_SYSCALL(setns),
        KILL_SYSCALL(swapon),
        KILL_SYSCALL(swapoff),
        KILL_SYSCALL(reboot), // !
        KILL_SYSCALL(sethostname),
        KILL_SYSCALL(setdomainname),
        KILL_SYSCALL(tkill),
        KILL_SYSCALL(mbind), // vm bits
        KILL_SYSCALL(set_mempolicy), // vm bits
        KILL_SYSCALL(get_mempolicy), // vm bits
        KILL_SYSCALL(kexec_load),
        KILL_SYSCALL(add_key),     // kernel keyring
        KILL_SYSCALL(request_key), // kernel keyring
        KILL_SYSCALL(keyctl),      // kernel keyring
#ifdef __NR_inotify_init
        KILL_SYSCALL(inotify_init),
#endif
        KILL_SYSCALL(inotify_add_watch),
        KILL_SYSCALL(inotify_rm_watch),
        KILL_SYSCALL(unshare),
        KILL_SYSCALL(splice),
        KILL_SYSCALL(tee),
        KILL_SYSCALL(vmsplice), // vm bits
        KILL_SYSCALL(move_pages), // vm bits
        KILL_SYSCALL(accept4), // server sockets
        KILL_SYSCALL(inotify_init1),
        KILL_SYSCALL(perf_event_open), // profiling
        KILL_SYSCALL(fanotify_init),
        KILL_SYSCALL(fanotify_mark),
#ifdef __NR_seccomp
        KILL_SYSCALL(seccomp), // no further fiddling
#endif
#ifdef __NR_bpf
        KILL_SYSCALL(bpf),     // no further fiddling
#endif

        // allow the rest.
        BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_ALLOW)
    };

    struct sock_fprog filter = {
        N_ELEMENTS(filterCode), // length
        filterCode
    };

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
    {
        LOG_ERR("Cannot turn off acquisition of new privileges for us & children");
        return false;
    }
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &filter))
    {
        LOG_ERR("Failed to install seccomp syscall filter");
        return false;
    }

    // Trap, log, and exit on failure
    struct sigaction action;

    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = handleSysSignal;

    sigaction(SIGSYS, &action, nullptr);

    LOG_TRC("Install seccomp filter successfully.");

    return true;
#else // DISABLE_SECCOMP == 0
     LOG_WRN("Warning: this binary was compiled with disabled seccomp-bpf.");
     return true;
#endif // DISABLE_SECCOMP == 0
}

} // namespace Seccomp

namespace Rlimit {

void setRLimit(rlim_t confLim, int resource, const std::string& resourceText,
               const std::string& unitText)
{
    rlim_t lim = confLim;
    if (lim <= 0)
        lim = RLIM_INFINITY;
    const std::string limTextWithUnit((lim == RLIM_INFINITY) ? "unlimited" : std::to_string(lim) + ' ' + unitText);
    if (resource == RLIMIT_FSIZE)
    {
        // Without this, a write past the limit terminates the kit with
        // SIGXFSZ (default action: core dump). Ignoring the signal leaves
        // write(2) returning EFBIG so LibreOffice's normal disk-full path
        // can surface the error to the client.
        std::signal(SIGXFSZ, SIG_IGN);
    }

    rlimit rlim = { lim, lim };
    if (setrlimit(resource, &rlim) != 0)
    {
        // Infinity is the default, and it's not an error if we fail to set it
        // as increasing limits are not valid. Error when the increase is intentional.
        if (lim != RLIM_INFINITY)
            LOG_SYS("Failed to set " << resourceText << " to " << limTextWithUnit << '.');
        else
            LOG_DBG("Failed to set " << resourceText << " to " << limTextWithUnit << '.');
    }

    if (getrlimit(resource, &rlim) == 0)
    {
        const std::string setLimTextWithUnit((rlim.rlim_max == RLIM_INFINITY)
                                                 ? "unlimited"
                                                 : std::to_string(rlim.rlim_max) + ' ' + unitText);
        LOG_INF(resourceText << " is " << setLimTextWithUnit << " after setting it to "
                             << limTextWithUnit << '.');
    }
    else
        LOG_SYS("Failed to get " << resourceText << " after trying to set it to "
                                 << limTextWithUnit);
}

bool handleSetrlimitCommand(const StringVector& tokens)
{
    if (tokens.size() == 3 && tokens.equals(0, "setconfig"))
    {
        if (tokens.equals(1, "limit_virt_mem_mb"))
        {
            setRLimit(NumUtil::stoi(tokens[2]) * 1024 * 1024, RLIMIT_AS, "RLIMIT_AS", "bytes");
        }
        else if (tokens.equals(1, "limit_stack_mem_kb"))
        {
            setRLimit(NumUtil::stoi(tokens[2]) * 1024, RLIMIT_STACK, "RLIMIT_STACK", "bytes");
        }
        else if (tokens.equals(1, "limit_file_size_mb"))
        {
            setRLimit(NumUtil::stoi(tokens[2]) * 1024 * 1024, RLIMIT_FSIZE, "RLIMIT_FSIZE",
                      "bytes");
        }
        else if (tokens.equals(1, "limit_num_open_files"))
        {
            setRLimit(NumUtil::stoi(tokens[2]), RLIMIT_NOFILE, "RLIMIT_NOFILE", "files");
        }
        else
            return false;

        return true;
    }
    return false;
}

} // namespace Rlimit

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
