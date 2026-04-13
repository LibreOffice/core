#!/usr/bin/env bash
#
# Wrapper for git to handle some common operations
#

if [ -n "$g_debug" ] ; then
    set -x
fi

pushd $(dirname $0) > /dev/null
if [ -f ${BUILDDIR}/config_host.mk ] ; then
    # we are in the SRCDIR
    SRC_ROOT=$(< ${BUILDDIR}/config_host.mk grep -a SRC_ROOT | sed -e "s/.*=//")
else
    SRC_ROOT=$(pwd)
fi
popd > /dev/null

COREDIR="$SRC_ROOT"

usage()
{
    git
    echo
    echo "Usage: g [options] [git (checkout|fetch|gc|grep|pull|push|reset) [git options/args..]]"
    echo ""
    echo " -z restore the git hooks and do other sanity checks"
}

refresh_create_link()
{
    local hook_name=$1
    local hook=$2
    local lnarg=$3

    # if it doesn't exist or is neither a symlink nor sharing the same inode (hardlink)
    if [ ! -e "${hook?}" ] || [ ! \( -L "${hook?}" -o "${hook_name}" -ef "${hook?}" \) ] ; then
        rm -f "${hook?}"
        ln -f $lnarg "${hook_name}" "${hook?}"
    fi
}

refresh_all_hooks()
{
    local hook_name
    local hook
    local gitbash
    local lnarg

    pushd "${COREDIR?}" > /dev/null

    # it is 'GIT for Windows'
    gitbash=$(echo $OSTYPE | grep -ic msys)

    # git-bash/MSYS doesn't create symlinks by default, and "real" symlinks are restricted to
    # Admin-mode or when devmode is activated, junction points as fallback would work for bash/
    # regular use but not when git tries to spawn them, similar for plain windows shortcuts (worse
    # because running the hooks will fail silently/they'd be inactive)
    # ln -s without setting MSYS to contain winsymlinks:{lnk,native,nativestrict,sys} to force one
    # of the other modes described above will do plain copies.
    # So in case of git-bash use hardlinks since those work just fine, everywhere else use symlinks
    if [ $gitbash -ne 1 ]; then
        lnarg="-s"
    fi
    # There's no ".git" e.g. in a secondary worktree
    if [ -d ".git" ]; then
        for hook_name in "${COREDIR?}/.git-hooks"/* ; do
            hook=".git/hooks/${hook_name##*/}"
            refresh_create_link "${hook_name}" "${hook?}" "$lnarg"
        done
    fi

    popd > /dev/null

}

set_push_url()
{
    pushd "${COREDIR?}" > /dev/null
    echo "setting up push url for core"
    git config remote.origin.pushurl "ssh://${PUSH_USER}logerrit/core"
    popd > /dev/null
}

# no params, no action
if [ "$#" -eq "0" ] ; then
    usage
fi


if [ ! "$(type -p git)" ]; then
    echo "Cannot find the git binary! Is git installed and is in PATH?"
    exit 1
fi


# extra params for some commands, like log
EXTRA=
COMMAND="$1"
PAGER=
PUSH_USER=
KEEP_GOING=


while [ "${COMMAND:0:1}" = "-" ] ; do
    case "$COMMAND" in
        -f )KEEP_GOING="||:"
            ;;
        -z)
            refresh_all_hooks
            exit 0;
            ;;
        --set-push-urls)
            shift
            PUSH_USER="$1"
            if [ -n "${PUSH_USER}" ] ; then
                PUSH_USER="${PUSH_USER}@"
            fi
            set_push_url
            exit 0;
            ;;
        -*)
            echo "option: $COMMAND not supported" 1>&2
            exit 1
    esac
    shift
    COMMAND="$1"
done

shift

case "$COMMAND" in
    branch)
        git branch "$@"
        ;;
    checkout)
        git checkout "$@"
        ;;
    fetch)
        git fetch "$@"
        ;;
    gc)
        git gc "$@"
        ;;
    grep)
        git grep "$@"
        ;;
    pull)
        git pull "$@" && refresh_all_hooks
        ;;
    push)
        git push "$@"
        ;;
    reset)
        git reset "$@"
        ;;
    tag)
        git tag "$@"
        ;;
    "")
        ;;
    *)
        echo "./g does not support command: $COMMAND" 1>&2
        exit 1;
        ;;
esac

exit $?

# vi:set shiftwidth=4 expandtab:
