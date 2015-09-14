/*************************************************************************

   Copyright 2011 Yuri Dario <mc6530@mclink.it>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 ************************************************************************/

#ifndef _SVPM_H
#define _SVPM_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __ZTC__
#define _Seg16  _far16
#define _Far16  _far16
#define _System _syscall
#define _Pascal _pascal
#define _Cdecl  _cdecl
#endif

#define BOOL            PM_BOOL
#define BYTE            PM_BYTE
#define ADDRESS         PM_ADDRESS  // YD xmloff
#define LINE            PM_LINE     // YD basic
#define CM_ERROR        PM_CM_ERROR // YD automation
#define CURSOR_FRAME    PM_CURSOR_FRAME // YD binfilter
#define POLYGON     PM_POLYGON  // YD sd
#define FIXED       PM_FIXED    // YD oox
#define DATETIME    PM_DATETIME // YD oox ooo320
#define RGB_RED     PM_RGB_RED  // YD vcl
#define RGB_GREEN   PM_RGB_GREEN    // YD vcl
#define RGB_BLUE    PM_RGB_BLUE // YD vcl
#define RGB     PM_RGB      // YD rsc bison 2.3

#define RGB_RED     PM_RGB_RED  // YD xmlhelp
#define RGB_BLUE    PM_RGB_BLUE // YD xmlhelp
#define RGB_GREEN   PM_RGB_GREEN    // YD xmlhelp
#define CURSOR_FRAME    PM_CURSOR_FRAME // YD xmlhelp
#define CM_ERROR    PM_CM_ERROR // YD xmlhelp
#define ADDRESS     PM_ADDRESS  // YD xmlhelp
#define COMMENT     PM_COMMENT  // YD offuh

#define INCL_PM
#define INCL_DOSSEMAPHORES

//yd 26/03/2006 OOo hack
#define OS2EMX_PLAIN_CHAR

#include <os2.h>
#include <unikbd.h>
#ifdef VCL_OS2
#include <pmbidi.h>
#endif

#undef BOOL
#undef BYTE
#undef ADDRESS
#undef LINE
#undef CM_ERROR
#undef CURSOR_FRAME
#undef POLYGON
#undef FIXED
#undef DATETIME
#undef RGB_RED
#undef RGB_GREEN
#undef RGB_BLUE
#undef RGB
#undef RGB_RED
#undef RGB_BLUE
#undef CURSOR_FRAME
#undef ADDRESS
#undef RGB_GREEN
#undef CM_ERROR
#undef COMMENT

#ifdef __cplusplus
}
#endif

#endif // _SVPM_H
