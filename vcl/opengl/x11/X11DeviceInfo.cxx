/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "opengl/x11/X11DeviceInfo.hxx"

#include <vcl/opengl/glxtest.hxx>
#include <rtl/ustring.hxx>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/utsname.h>

namespace glx {

static int glxtest_pipe = 0;

static pid_t glxtest_pid = 0;

}

pid_t* getGlxPid()
{
    return &glx::glxtest_pid;
}

int* getGlxPipe()
{
    return &glx::glxtest_pipe;
}

namespace {

const char*
strspnp_wrapper(const char* aDelims, const char* aStr)
{
  const char* d;
  do
  {
      for (d = aDelims; *d != '\0'; ++d)
      {
          if (*aStr == *d)
          {
              ++aStr;
              break;
          }
      }
  } while (*d);

  return aStr;
}

char* strtok_wrapper(const char* aDelims, char** aStr)
{
    if (!*aStr)
    {
        return nullptr;
    }

    char* ret = const_cast<char*>(strspnp_wrapper(aDelims, *aStr));

    if (!*ret)
    {
        *aStr = ret;
        return nullptr;
    }

    char* i = ret;
    do
    {
        for (const char* d = aDelims; *d != '\0'; ++d)
        {
            if (*i == *d) {
                *i = '\0';
                *aStr = ++i;
                return ret;
            }
        }
        ++i;
    } while (*i);

    *aStr = nullptr;
    return ret;
}

uint64_t version(uint32_t major, uint32_t minor, uint32_t revision = 0)
{
    return (uint64_t(major) << 32) + (uint64_t(minor) << 16) + uint64_t(revision);
}

}

X11OpenGLDeviceInfo::X11OpenGLDeviceInfo():
    mbIsMesa(false),
    mbIsNVIDIA(false),
    mbIsFGLRX(false),
    mbIsNouveau(false),
    mbIsIntel(false),
    mbIsOldSwrast(false),
    mbIsLlvmpipe(false),
    mbHasTextureFromPixmap(false),
    mnGLMajorVersion(0),
    mnMajorVersion(0),
    mnMinorVersion(0),
    mnRevisionVersion(0)
{
    GetData();
}

X11OpenGLDeviceInfo::~X11OpenGLDeviceInfo()
{
}

void X11OpenGLDeviceInfo::GetData()
{
    if (!glx::glxtest_pipe)
       return;

    // to understand this function, see bug moz#639842. We retrieve the OpenGL driver information in a
    // separate process to protect against bad drivers.
    enum { buf_size = 1024 };
    char buf[buf_size];
    ssize_t bytesread = read(glx::glxtest_pipe,
            &buf,
            buf_size-1); // -1 because we'll append a zero
    close(glx::glxtest_pipe);
    glx::glxtest_pipe = 0;

    // bytesread < 0 would mean that the above read() call failed.
    // This should never happen. If it did, the outcome would be to blacklist anyway.
    if (bytesread < 0)
        bytesread = 0;

    // let buf be a zero-terminated string
    buf[bytesread] = 0;

    // Wait for the glxtest process to finish. This serves 2 purposes:
    // * avoid having a zombie glxtest process laying around
    // * get the glxtest process status info.
    int glxtest_status = 0;
    bool wait_for_glxtest_process = true;
    bool waiting_for_glxtest_process_failed = false;
    int waitpid_errno = 0;
    while(wait_for_glxtest_process)
    {
        wait_for_glxtest_process = false;
        if (waitpid(glx::glxtest_pid, &glxtest_status, 0) == -1)
        {
            waitpid_errno = errno;
            if (waitpid_errno == EINTR)
            {
                wait_for_glxtest_process = true;
            }
            else
            {
                // Bug moz#718629
                // ECHILD happens when the glxtest process got reaped got reaped after a PR_CreateProcess
                // as per bug moz#227246. This shouldn't matter, as we still seem to get the data
                // from the pipe, and if we didn't, the outcome would be to blacklist anyway.
                waiting_for_glxtest_process_failed = (waitpid_errno != ECHILD);
            }
        }
    }

    bool exited_with_error_code = !waiting_for_glxtest_process_failed &&
        WIFEXITED(glxtest_status) &&
        WEXITSTATUS(glxtest_status) != EXIT_SUCCESS;
    bool received_signal = !waiting_for_glxtest_process_failed &&
        WIFSIGNALED(glxtest_status);

    bool error = waiting_for_glxtest_process_failed || exited_with_error_code || received_signal;

    OString textureFromPixmap;
    OString *stringToFill = nullptr;
    char *bufptr = buf;
    if (!error)
    {
        while(true)
        {
            char *line = strtok_wrapper("\n", &bufptr);
            if (!line)
                break;
            if (stringToFill) {
                *stringToFill = OString(line);
                stringToFill = nullptr;
            }
            else if(!strcmp(line, "VENDOR"))
                stringToFill = &maVendor;
            else if(!strcmp(line, "RENDERER"))
                stringToFill = &maRenderer;
            else if(!strcmp(line, "VERSION"))
                stringToFill = &maVersion;
            else if(!strcmp(line, "TFP"))
                stringToFill = &textureFromPixmap;
        }
    }

    if (!strcmp(textureFromPixmap.getStr(), "TRUE"))
        mbHasTextureFromPixmap = true;

    // only useful for Linux kernel version check for FGLRX driver.
    // assumes X client == X server, which is sad.
    struct utsname unameobj;
    if (!uname(&unameobj))
    {
        maOS = OString(unameobj.sysname);
        maOSRelease = OString(unameobj.release);
    }

    // determine the major OpenGL version. That's the first integer in the version string.
    mnGLMajorVersion = strtol(maVersion.getStr(), nullptr, 10);

    // determine driver type (vendor) and where in the version string
    // the actual driver version numbers should be expected to be found (whereToReadVersionNumbers)
    const char *whereToReadVersionNumbers = nullptr;
    const char *Mesa_in_version_string = strstr(maVersion.getStr(), "Mesa");
    if (Mesa_in_version_string)
    {
        mbIsMesa = true;
        // with Mesa, the version string contains "Mesa major.minor" and that's all the version information we get:
        // there is no actual driver version info.
        whereToReadVersionNumbers = Mesa_in_version_string + strlen("Mesa");
        if (strcasestr(maVendor.getStr(), "nouveau"))
            mbIsNouveau = true;
        if (strcasestr(maRenderer.getStr(), "intel")) // yes, intel is in the renderer string
            mbIsIntel = true;
        if (strcasestr(maRenderer.getStr(), "llvmpipe"))
            mbIsLlvmpipe = true;
        if (strcasestr(maRenderer.getStr(), "software rasterizer"))
            mbIsOldSwrast = true;
    }
    else if (strstr(maVendor.getStr(), "NVIDIA Corporation"))
    {
        mbIsNVIDIA = true;
        // with the NVIDIA driver, the version string contains "NVIDIA major.minor"
        // note that here the vendor and version strings behave differently, that's why we don't put this above
        // alongside Mesa_in_version_string.
        const char *NVIDIA_in_version_string = strstr(maVersion.getStr(), "NVIDIA");
        if (NVIDIA_in_version_string)
            whereToReadVersionNumbers = NVIDIA_in_version_string + strlen("NVIDIA");
    }
    else if (strstr(maVendor.getStr(), "ATI Technologies Inc"))
    {
        mbIsFGLRX = true;
        // with the FGLRX driver, the version string only gives a OpenGL version :/ so let's return that.
        // that can at least give a rough idea of how old the driver is.
        whereToReadVersionNumbers = maVersion.getStr();
    }

    // read major.minor version numbers of the driver (not to be confused with the OpenGL version)
    if (whereToReadVersionNumbers)
    {
        // copy into writable buffer, for tokenization
        strncpy(buf, whereToReadVersionNumbers, buf_size-1);
        buf[buf_size-1] = 0;
        bufptr = buf;

        // now try to read major.minor version numbers. In case of failure, gracefully exit: these numbers have
        // been initialized as 0 anyways
        char *token = strtok_wrapper(".", &bufptr);
        if (token)
        {
            mnMajorVersion = strtol(token, nullptr, 10);
            token = strtok_wrapper(".", &bufptr);
            if (token)
            {
                mnMinorVersion = strtol(token, nullptr, 10);
                token = strtok_wrapper(".", &bufptr);
                if (token)
                    mnRevisionVersion = strtol(token, nullptr, 10);
            }
        }
    }
}

bool X11OpenGLDeviceInfo::isDeviceBlocked()
{
    // don't even try to use OpenGL 1.x
    if (mnGLMajorVersion == 1)
        return true;

    SAL_INFO("vcl.opengl", "Vendor: " << maVendor);
    SAL_INFO("vcl.opengl", "Renderer: " << maRenderer);
    SAL_INFO("vcl.opengl", "Version: " << maVersion);
    SAL_INFO("vcl.opengl", "OS: " << maOS);
    SAL_INFO("vcl.opengl", "OSRelease: " << maOSRelease);

    if (mbIsMesa)
    {
        if (mbIsNouveau && version(mnMajorVersion, mnMinorVersion) < version(8,0))
        {
            SAL_WARN("vcl.opengl", "blocked driver version: old nouveau driver (requires mesa 8.0+)");
            return true;
        }
        else if (version(mnMajorVersion, mnMinorVersion, mnRevisionVersion) < version(7,10,3))
        {
            SAL_WARN("vcl.opengl", "blocked driver version: requires at least mesa 7.10.3");
            return true;
        }
        else if (mbIsIntel && version(mnMajorVersion, mnMinorVersion, mnRevisionVersion) == version(9,0,2))
        {
            SAL_WARN("vcl.opengl", "blocked driver version: my broken intel driver Mesa 9.0.2");
            return true;
        }
        else if (mbIsOldSwrast)
        {
            SAL_WARN("vcl.opengl", "blocked driver version: software rasterizer");
            return true;
        }
        else if (mbIsLlvmpipe && version(mnMajorVersion, mnMinorVersion) < version(9, 1))
        {
            // bug moz#791905, Mesa bug 57733, fixed in Mesa 9.1 according to
            // https://bugs.freedesktop.org/show_bug.cgi?id=57733#c3
            SAL_WARN("vcl.opengl", "blocked driver version: fdo#57733");
            return true;
        }
    }
    else if (mbIsNVIDIA)
    {
        if (version(mnMajorVersion, mnMinorVersion, mnRevisionVersion) < version(257,21))
        {
            SAL_WARN("vcl.opengl", "blocked driver version: nvidia requires at least 257.21");
            return true;
        }
    }
    else if (mbIsFGLRX)
    {
        // FGLRX does not report a driver version number, so we have the OpenGL version instead.
        // by requiring OpenGL 3, we effectively require recent drivers.
        if (version(mnMajorVersion, mnMinorVersion, mnRevisionVersion) < version(3, 0))
        {
            SAL_WARN("vcl.opengl", "blocked driver version: require at least OpenGL 3 for fglrx");
            return true;
        }
        // Bug moz#724640: FGLRX + Linux 2.6.32 is a crashy combo
        bool unknownOS = maOS.isEmpty() || maOSRelease.isEmpty();
        bool badOS = maOS.indexOf("Linux") != -1 &&
            maOSRelease.indexOf("2.6.32") != -1;
        if (unknownOS || badOS)
        {
            SAL_WARN("vcl.opengl", "blocked OS version with fglrx");
            return true;
        }
    }
    else
    {
        // like on windows, let's block unknown vendors. Think of virtual machines.
        // Also, this case is hit whenever the GLXtest probe failed to get driver info or crashed.
        SAL_WARN("vcl.opengl", "unknown vendor => blocked");
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
