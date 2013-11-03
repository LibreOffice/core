/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

/*
 *  function call mechanics needed by platform specific glue code.
 */


#ifndef _NPUPP_H_
#define _NPUPP_H_

#ifndef GENERATINGCFM
#define GENERATINGCFM 0
#endif

#ifndef _NPAPI_H_
#include "npapi.h"
#endif

#include "jri.h"

/******************************************************************************************
   plug-in function table macros
             for each function in and out of the plugin API we define
                    typedef NPP_FooUPP
                    #define NewNPP_FooProc
                    #define CallNPP_FooProc
            for mac, define the UPP magic for PPC/68K calling
 *******************************************************************************************/


/* NPP_Initialize */

#ifndef TARGET_RT_MAC_CFM
#define TARGET_RT_MAC_CFM 0
#endif

#define _NPUPP_USE_UPP_ (TARGET_RT_MAC_CFM && !TARGET_API_MAC_CARBON)

#if _NPUPP_USE_UPP_
typedef UniversalProcPtr NPP_InitializeUPP;

enum {
    uppNPP_InitializeProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(0))
        | RESULT_SIZE(SIZE_CODE(0))
};

#define NewNPP_InitializeProc(FUNC)     \
        (NPP_InitializeUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPP_InitializeProcInfo, GetCurrentArchitecture())
#define CallNPP_InitializeProc(FUNC)        \
        (void)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPP_InitializeProcInfo)

#else

typedef void (* NP_LOADDS NPP_InitializeUPP)(void);
#define NewNPP_InitializeProc(FUNC)     \
        ((NPP_InitializeUPP) (FUNC))
#define CallNPP_InitializeProc(FUNC)        \
        (*(FUNC))()

#endif


/* NPP_Shutdown */

#if _NPUPP_USE_UPP_
typedef UniversalProcPtr NPP_ShutdownUPP;

enum {
    uppNPP_ShutdownProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(0))
        | RESULT_SIZE(SIZE_CODE(0))
};

#define NewNPP_ShutdownProc(FUNC)       \
        (NPP_ShutdownUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPP_ShutdownProcInfo, GetCurrentArchitecture())
#define CallNPP_ShutdownProc(FUNC)      \
        (void)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPP_ShutdownProcInfo)

#else

typedef void (* NP_LOADDS NPP_ShutdownUPP)(void);
#define NewNPP_ShutdownProc(FUNC)       \
        ((NPP_ShutdownUPP) (FUNC))
#define CallNPP_ShutdownProc(FUNC)      \
        (*(FUNC))()

#endif


/* NPP_New */

#if _NPUPP_USE_UPP_
typedef UniversalProcPtr NPP_NewUPP;

enum {
    uppNPP_NewProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPMIMEType)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(uint16_t)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(int16_t)))
        | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(char **)))
        | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(char **)))
        | STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(NPSavedData *)))
        | RESULT_SIZE(SIZE_CODE(sizeof(NPError)))
};

#define NewNPP_NewProc(FUNC)        \
        (NPP_NewUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPP_NewProcInfo, GetCurrentArchitecture())
#define CallNPP_NewProc(FUNC, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) \
        (NPError)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPP_NewProcInfo, \
                                   (ARG1), (ARG2), (ARG3), (ARG4), (ARG5), (ARG6), (ARG7))
#else

typedef NPError (* NP_LOADDS NPP_NewUPP)(NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc, char* argn[], char* argv[], NPSavedData* saved);
#define NewNPP_NewProc(FUNC)        \
        ((NPP_NewUPP) (FUNC))
#define CallNPP_NewProc(FUNC, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)     \
        (*(FUNC))((ARG1), (ARG2), (ARG3), (ARG4), (ARG5), (ARG6), (ARG7))

#endif


/* NPP_Destroy */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPP_DestroyUPP;
enum {
    uppNPP_DestroyProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPSavedData **)))
        | RESULT_SIZE(SIZE_CODE(sizeof(NPError)))
};
#define NewNPP_DestroyProc(FUNC)        \
        (NPP_DestroyUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPP_DestroyProcInfo, GetCurrentArchitecture())
#define CallNPP_DestroyProc(FUNC, ARG1, ARG2)       \
        (NPError)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPP_DestroyProcInfo, (ARG1), (ARG2))
#else

typedef NPError (* NP_LOADDS NPP_DestroyUPP)(NPP instance, NPSavedData** save);
#define NewNPP_DestroyProc(FUNC)        \
        ((NPP_DestroyUPP) (FUNC))
#define CallNPP_DestroyProc(FUNC, ARG1, ARG2)       \
        (*(FUNC))((ARG1), (ARG2))

#endif


/* NPP_SetWindow */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPP_SetWindowUPP;
enum {
    uppNPP_SetWindowProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPWindow *)))
        | RESULT_SIZE(SIZE_CODE(sizeof(NPError)))
};
#define NewNPP_SetWindowProc(FUNC)      \
        (NPP_SetWindowUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPP_SetWindowProcInfo, GetCurrentArchitecture())
#define CallNPP_SetWindowProc(FUNC, ARG1, ARG2)     \
        (NPError)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPP_SetWindowProcInfo, (ARG1), (ARG2))

#else

typedef NPError (* NP_LOADDS NPP_SetWindowUPP)(NPP instance, NPWindow* window);
#define NewNPP_SetWindowProc(FUNC)      \
        ((NPP_SetWindowUPP) (FUNC))
#define CallNPP_SetWindowProc(FUNC, ARG1, ARG2)     \
        (*(FUNC))((ARG1), (ARG2))

#endif


/* NPP_NewStream */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPP_NewStreamUPP;
enum {
    uppNPP_NewStreamProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPMIMEType)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(NPStream *)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(NPBool)))
        | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(uint16_t *)))
        | RESULT_SIZE(SIZE_CODE(sizeof(NPError)))
};
#define NewNPP_NewStreamProc(FUNC)      \
        (NPP_NewStreamUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPP_NewStreamProcInfo, GetCurrentArchitecture())
#define CallNPP_NewStreamProc(FUNC, ARG1, ARG2, ARG3, ARG4, ARG5)       \
        (NPError)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPP_NewStreamProcInfo, (ARG1), (ARG2), (ARG3), (ARG4), (ARG5))
#else

typedef NPError (* NP_LOADDS NPP_NewStreamUPP)(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16_t* stype);
#define NewNPP_NewStreamProc(FUNC)      \
        ((NPP_NewStreamUPP) (FUNC))
#define CallNPP_NewStreamProc(FUNC, ARG1, ARG2, ARG3, ARG4, ARG5) \
        (*(FUNC))((ARG1), (ARG2), (ARG3), (ARG4), (ARG5))
#endif


/* NPP_DestroyStream */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPP_DestroyStreamUPP;
enum {
    uppNPP_DestroyStreamProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPStream *)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(NPReason)))
        | RESULT_SIZE(SIZE_CODE(sizeof(NPError)))
};
#define NewNPP_DestroyStreamProc(FUNC)      \
        (NPP_DestroyStreamUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPP_DestroyStreamProcInfo, GetCurrentArchitecture())
#define CallNPP_DestroyStreamProc(FUNC,  NPParg, NPStreamPtr, NPReasonArg)      \
        (NPError)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPP_DestroyStreamProcInfo, (NPParg), (NPStreamPtr), (NPReasonArg))

#else

typedef NPError (* NP_LOADDS NPP_DestroyStreamUPP)(NPP instance, NPStream* stream, NPReason reason);
#define NewNPP_DestroyStreamProc(FUNC)      \
        ((NPP_DestroyStreamUPP) (FUNC))
#define CallNPP_DestroyStreamProc(FUNC,  NPParg, NPStreamPtr, NPReasonArg)      \
        (*(FUNC))((NPParg), (NPStreamPtr), (NPReasonArg))

#endif


/* NPP_WriteReady */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPP_WriteReadyUPP;
enum {
    uppNPP_WriteReadyProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPStream *)))
        | RESULT_SIZE(SIZE_CODE(sizeof(int32_t)))
};
#define NewNPP_WriteReadyProc(FUNC)     \
        (NPP_WriteReadyUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPP_WriteReadyProcInfo, GetCurrentArchitecture())
#define CallNPP_WriteReadyProc(FUNC,  NPParg, NPStreamPtr)      \
        (int32_t)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPP_WriteReadyProcInfo, (NPParg), (NPStreamPtr))

#else

typedef int32_t (* NP_LOADDS NPP_WriteReadyUPP)(NPP instance, NPStream* stream);
#define NewNPP_WriteReadyProc(FUNC)     \
        ((NPP_WriteReadyUPP) (FUNC))
#define CallNPP_WriteReadyProc(FUNC,  NPParg, NPStreamPtr)      \
        (*(FUNC))((NPParg), (NPStreamPtr))

#endif


/* NPP_Write */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPP_WriteUPP;
enum {
    uppNPP_WriteProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPStream *)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(int32_t)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(int32_t)))
        | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(void*)))
        | RESULT_SIZE(SIZE_CODE(sizeof(int32_t)))
};
#define NewNPP_WriteProc(FUNC)      \
        (NPP_WriteUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPP_WriteProcInfo, GetCurrentArchitecture())
#define CallNPP_WriteProc(FUNC,  NPParg, NPStreamPtr, offsetArg, lenArg, bufferPtr)     \
        (int32_t)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPP_WriteProcInfo, (NPParg), (NPStreamPtr), (offsetArg), (lenArg), (bufferPtr))

#else

typedef int32_t (* NP_LOADDS NPP_WriteUPP)(NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buffer);
#define NewNPP_WriteProc(FUNC)      \
        ((NPP_WriteUPP) (FUNC))
#define CallNPP_WriteProc(FUNC,  NPParg, NPStreamPtr, offsetArg, lenArg, bufferPtr)     \
        (*(FUNC))((NPParg), (NPStreamPtr), (offsetArg), (lenArg), (bufferPtr))

#endif


/* NPP_StreamAsFile */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPP_StreamAsFileUPP;
enum {
    uppNPP_StreamAsFileProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPStream *)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(const char *)))
        | RESULT_SIZE(SIZE_CODE(0))
};
#define NewNPP_StreamAsFileProc(FUNC)       \
        (NPP_StreamAsFileUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPP_StreamAsFileProcInfo, GetCurrentArchitecture())
#define CallNPP_StreamAsFileProc(FUNC, ARG1, ARG2, ARG3)        \
        (void)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPP_StreamAsFileProcInfo, (ARG1), (ARG2), (ARG3))

#else

typedef void (* NP_LOADDS NPP_StreamAsFileUPP)(NPP instance, NPStream* stream, const char* fname);
#define NewNPP_StreamAsFileProc(FUNC)       \
        ((NPP_StreamAsFileUPP) (FUNC))
#define CallNPP_StreamAsFileProc(FUNC,  ARG1, ARG2, ARG3)       \
        (*(FUNC))((ARG1), (ARG2), (ARG3))
#endif


/* NPP_Print */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPP_PrintUPP;
enum {
    uppNPP_PrintProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPPrint *)))
        | RESULT_SIZE(SIZE_CODE(0))
};
#define NewNPP_PrintProc(FUNC)      \
        (NPP_PrintUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPP_PrintProcInfo, GetCurrentArchitecture())
#define CallNPP_PrintProc(FUNC,  NPParg, voidPtr)       \
        (void)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPP_PrintProcInfo, (NPParg), (voidPtr))

#else

typedef void (* NP_LOADDS NPP_PrintUPP)(NPP instance, NPPrint* platformPrint);
#define NewNPP_PrintProc(FUNC)      \
        ((NPP_PrintUPP) (FUNC))
#define CallNPP_PrintProc(FUNC,  NPParg, NPPrintArg)        \
        (*(FUNC))((NPParg), (NPPrintArg))

#endif


/* NPP_HandleEvent */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPP_HandleEventUPP;
enum {
    uppNPP_HandleEventProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(void *)))
        | RESULT_SIZE(SIZE_CODE(sizeof(int16_t)))
};
#define NewNPP_HandleEventProc(FUNC)        \
        (NPP_HandleEventUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPP_HandleEventProcInfo, GetCurrentArchitecture())
#define CallNPP_HandleEventProc(FUNC,  NPParg, voidPtr)     \
        (int16_t)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPP_HandleEventProcInfo, (NPParg), (voidPtr))

#else

typedef int16_t (* NP_LOADDS NPP_HandleEventUPP)(NPP instance, void* event);
#define NewNPP_HandleEventProc(FUNC)        \
        ((NPP_HandleEventUPP) (FUNC))
#define CallNPP_HandleEventProc(FUNC,  NPParg, voidPtr)     \
        (*(FUNC))((NPParg), (voidPtr))

#endif


/* NPP_URLNotify */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPP_URLNotifyUPP;
enum {
    uppNPP_URLNotifyProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(const char*)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(NPReason)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(void*)))
        | RESULT_SIZE(SIZE_CODE(SIZE_CODE(0)))
};
#define NewNPP_URLNotifyProc(FUNC)      \
        (NPP_URLNotifyUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPP_URLNotifyProcInfo, GetCurrentArchitecture())
#define CallNPP_URLNotifyProc(FUNC,  ARG1, ARG2, ARG3, ARG4)        \
        (void)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPP_URLNotifyProcInfo, (ARG1), (ARG2), (ARG3), (ARG4))

#else

typedef void (* NP_LOADDS NPP_URLNotifyUPP)(NPP instance, const char* url, NPReason reason, void* notifyData);
#define NewNPP_URLNotifyProc(FUNC)      \
        ((NPP_URLNotifyUPP) (FUNC))
#define CallNPP_URLNotifyProc(FUNC,  ARG1, ARG2, ARG3, ARG4)        \
        (*(FUNC))((ARG1), (ARG2), (ARG3), (ARG4))

#endif


/* NPP_GetValue */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPP_GetValueUPP;
enum {
    uppNPP_GetValueProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPPVariable)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(void *)))
        | RESULT_SIZE(SIZE_CODE(sizeof(NPError)))
};
#define NewNPP_GetValueProc(FUNC)       \
        (NPP_GetValueUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPP_GetValueProcInfo, GetCurrentArchitecture())
#define CallNPP_GetValueProc(FUNC, ARG1, ARG2, ARG3) \
        (NPError)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPP_GetValueProcInfo, (ARG1), (ARG2), (ARG3))
#else

typedef NPError (* NP_LOADDS NPP_GetValueUPP)(NPP instance, NPPVariable variable, void *ret_alue);
#define NewNPP_GetValueProc(FUNC)       \
        ((NPP_GetValueUPP) (FUNC))
#define CallNPP_GetValueProc(FUNC, ARG1, ARG2, ARG3)        \
        (*(FUNC))((ARG1), (ARG2), (ARG3))
#endif


/* NPP_SetValue */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPP_SetValueUPP;
enum {
    uppNPP_SetValueProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPNVariable)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(void *)))
        | RESULT_SIZE(SIZE_CODE(sizeof(NPError)))
};
#define NewNPP_SetValueProc(FUNC)       \
        (NPP_SetValueUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPP_SetValueProcInfo, GetCurrentArchitecture())
#define CallNPP_SetValueProc(FUNC, ARG1, ARG2, ARG3) \
        (NPError)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPP_SetValueProcInfo, (ARG1), (ARG2), (ARG3))
#else

typedef NPError (* NP_LOADDS NPP_SetValueUPP)(NPP instance, NPNVariable variable, void *ret_alue);
#define NewNPP_SetValueProc(FUNC)       \
        ((NPP_SetValueUPP) (FUNC))
#define CallNPP_SetValueProc(FUNC, ARG1, ARG2, ARG3)        \
        (*(FUNC))((ARG1), (ARG2), (ARG3))
#endif




/*
 *  Netscape entry points
 */


/* NPN_GetValue */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_GetValueUPP;
enum {
    uppNPN_GetValueProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPNVariable)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(void *)))
        | RESULT_SIZE(SIZE_CODE(sizeof(NPError)))
};
#define NewNPN_GetValueProc(FUNC)       \
        (NPN_GetValueUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_GetValueProcInfo, GetCurrentArchitecture())
#define CallNPN_GetValueProc(FUNC, ARG1, ARG2, ARG3) \
        (NPError)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_GetValueProcInfo, (ARG1), (ARG2), (ARG3))
#else

typedef NPError (* NP_LOADDS NPN_GetValueUPP)(NPP instance, NPNVariable variable, void *ret_alue);
#define NewNPN_GetValueProc(FUNC)       \
        ((NPN_GetValueUPP) (FUNC))
#define CallNPN_GetValueProc(FUNC, ARG1, ARG2, ARG3)        \
        (*(FUNC))((ARG1), (ARG2), (ARG3))
#endif


/* NPN_SetValue */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_SetValueUPP;
enum {
    uppNPN_SetValueProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPPVariable)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(void *)))
        | RESULT_SIZE(SIZE_CODE(sizeof(NPError)))
};
#define NewNPN_SetValueProc(FUNC)       \
        (NPN_SetValueUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_SetValueProcInfo, GetCurrentArchitecture())
#define CallNPN_SetValueProc(FUNC, ARG1, ARG2, ARG3) \
        (NPError)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_SetValueProcInfo, (ARG1), (ARG2), (ARG3))
#else

typedef NPError (* NP_LOADDS NPN_SetValueUPP)(NPP instance, NPPVariable variable, void *ret_alue);
#define NewNPN_SetValueProc(FUNC)       \
        ((NPN_SetValueUPP) (FUNC))
#define CallNPN_SetValueProc(FUNC, ARG1, ARG2, ARG3)        \
        (*(FUNC))((ARG1), (ARG2), (ARG3))
#endif


/* NPN_GetUrlNotify */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_GetURLNotifyUPP;
enum {
    uppNPN_GetURLNotifyProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(const char*)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(const char*)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(void*)))
        | RESULT_SIZE(SIZE_CODE(sizeof(NPError)))
};
#define NewNPN_GetURLNotifyProc(FUNC)       \
        (NPN_GetURLNotifyUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_GetURLNotifyProcInfo, GetCurrentArchitecture())
#define CallNPN_GetURLNotifyProc(FUNC, ARG1, ARG2, ARG3, ARG4) \
        (NPError)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_GetURLNotifyProcInfo, (ARG1), (ARG2), (ARG3), (ARG4))
#else

typedef NPError (* NP_LOADDS NPN_GetURLNotifyUPP)(NPP instance, const char* url, const char* window, void* notifyData);
#define NewNPN_GetURLNotifyProc(FUNC)       \
        ((NPN_GetURLNotifyUPP) (FUNC))
#define CallNPN_GetURLNotifyProc(FUNC, ARG1, ARG2, ARG3, ARG4)      \
        (*(FUNC))((ARG1), (ARG2), (ARG3), (ARG4))
#endif


/* NPN_PostUrlNotify */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_PostURLNotifyUPP;
enum {
    uppNPN_PostURLNotifyProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(const char*)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(const char*)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(uint32_t)))
        | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(const char*)))
        | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(NPBool)))
        | STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(void*)))
        | RESULT_SIZE(SIZE_CODE(sizeof(NPError)))
};
#define NewNPN_PostURLNotifyProc(FUNC)      \
        (NPN_PostURLNotifyUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_PostURLNotifyProcInfo, GetCurrentArchitecture())
#define CallNPN_PostURLNotifyProc(FUNC, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) \
        (NPError)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_PostURLNotifyProcInfo, (ARG1), (ARG2), (ARG3), (ARG4), (ARG5), (ARG6), (ARG7))
#else

typedef NPError (* NP_LOADDS NPN_PostURLNotifyUPP)(NPP instance, const char* url, const char* window, uint32_t len, const char* buf, NPBool file, void* notifyData);
#define NewNPN_PostURLNotifyProc(FUNC)      \
        ((NPN_PostURLNotifyUPP) (FUNC))
#define CallNPN_PostURLNotifyProc(FUNC, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) \
        (*(FUNC))((ARG1), (ARG2), (ARG3), (ARG4), (ARG5), (ARG6), (ARG7))
#endif


/* NPN_GetUrl */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_GetURLUPP;
enum {
    uppNPN_GetURLProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(const char*)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(const char*)))
        | RESULT_SIZE(SIZE_CODE(sizeof(NPError)))
};
#define NewNPN_GetURLProc(FUNC)     \
        (NPN_GetURLUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_GetURLProcInfo, GetCurrentArchitecture())
#define CallNPN_GetURLProc(FUNC, ARG1, ARG2, ARG3) \
        (NPError)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_GetURLProcInfo, (ARG1), (ARG2), (ARG3))
#else

typedef NPError (* NP_LOADDS NPN_GetURLUPP)(NPP instance, const char* url, const char* window);
#define NewNPN_GetURLProc(FUNC)     \
        ((NPN_GetURLUPP) (FUNC))
#define CallNPN_GetURLProc(FUNC, ARG1, ARG2, ARG3)      \
        (*(FUNC))((ARG1), (ARG2), (ARG3))
#endif


/* NPN_PostUrl */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_PostURLUPP;
enum {
    uppNPN_PostURLProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(const char*)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(const char*)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(uint32_t)))
        | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(const char*)))
        | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(NPBool)))
        | RESULT_SIZE(SIZE_CODE(sizeof(NPError)))
};
#define NewNPN_PostURLProc(FUNC)        \
        (NPN_PostURLUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_PostURLProcInfo, GetCurrentArchitecture())
#define CallNPN_PostURLProc(FUNC, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) \
        (NPError)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_PostURLProcInfo, (ARG1), (ARG2), (ARG3), (ARG4), (ARG5), (ARG6))
#else

typedef NPError (* NP_LOADDS NPN_PostURLUPP)(NPP instance, const char* url, const char* window, uint32_t len, const char* buf, NPBool file);
#define NewNPN_PostURLProc(FUNC)        \
        ((NPN_PostURLUPP) (FUNC))
#define CallNPN_PostURLProc(FUNC, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) \
        (*(FUNC))((ARG1), (ARG2), (ARG3), (ARG4), (ARG5), (ARG6))
#endif


/* NPN_RequestRead */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_RequestReadUPP;
enum {
    uppNPN_RequestReadProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPStream *)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPByteRange *)))
        | RESULT_SIZE(SIZE_CODE(sizeof(NPError)))
};
#define NewNPN_RequestReadProc(FUNC)        \
        (NPN_RequestReadUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_RequestReadProcInfo, GetCurrentArchitecture())
#define CallNPN_RequestReadProc(FUNC,  stream, range)       \
        (NPError)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_RequestReadProcInfo, (stream), (range))

#else

typedef NPError (* NP_LOADDS NPN_RequestReadUPP)(NPStream* stream, NPByteRange* rangeList);
#define NewNPN_RequestReadProc(FUNC)        \
        ((NPN_RequestReadUPP) (FUNC))
#define CallNPN_RequestReadProc(FUNC, stream, range)        \
        (*(FUNC))((stream), (range))

#endif


/* NPN_NewStream */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_NewStreamUPP;
enum {
    uppNPN_NewStreamProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPMIMEType)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(const char *)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(NPStream **)))
        | RESULT_SIZE(SIZE_CODE(sizeof(NPError)))
};
#define NewNPN_NewStreamProc(FUNC)      \
        (NPN_NewStreamUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_NewStreamProcInfo, GetCurrentArchitecture())
#define CallNPN_NewStreamProc(FUNC, npp, type, window, stream)      \
        (NPError)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_NewStreamProcInfo, (npp), (type), (window), (stream))

#else

typedef NPError (* NP_LOADDS NPN_NewStreamUPP)(NPP instance, NPMIMEType type, const char* window, NPStream** stream);
#define NewNPN_NewStreamProc(FUNC)      \
        ((NPN_NewStreamUPP) (FUNC))
#define CallNPN_NewStreamProc(FUNC, npp, type, window, stream)      \
        (*(FUNC))((npp), (type), (window), (stream))

#endif


/* NPN_Write */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_WriteUPP;
enum {
    uppNPN_WriteProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPStream *)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(int32_t)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(void*)))
        | RESULT_SIZE(SIZE_CODE(sizeof(int32_t)))
};
#define NewNPN_WriteProc(FUNC)      \
        (NPN_WriteUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_WriteProcInfo, GetCurrentArchitecture())
#define CallNPN_WriteProc(FUNC, npp, stream, len, buffer)       \
        (int32_t)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_WriteProcInfo, (npp), (stream), (len), (buffer))

#else

typedef int32_t (* NP_LOADDS NPN_WriteUPP)(NPP instance, NPStream* stream, int32_t len, void* buffer);
#define NewNPN_WriteProc(FUNC)      \
        ((NPN_WriteUPP) (FUNC))
#define CallNPN_WriteProc(FUNC, npp, stream, len, buffer)       \
        (*(FUNC))((npp), (stream), (len), (buffer))

#endif


/* NPN_DestroyStream */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_DestroyStreamUPP;
enum {
    uppNPN_DestroyStreamProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP )))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPStream *)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(NPReason)))
        | RESULT_SIZE(SIZE_CODE(sizeof(NPError)))
};
#define NewNPN_DestroyStreamProc(FUNC)      \
        (NPN_DestroyStreamUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_DestroyStreamProcInfo, GetCurrentArchitecture())
#define CallNPN_DestroyStreamProc(FUNC, npp, stream, reason)        \
        (NPError)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_DestroyStreamProcInfo, (npp), (stream), (reason))

#else

typedef NPError (* NP_LOADDS NPN_DestroyStreamUPP)(NPP instance, NPStream* stream, NPReason reason);
#define NewNPN_DestroyStreamProc(FUNC)      \
        ((NPN_DestroyStreamUPP) (FUNC))
#define CallNPN_DestroyStreamProc(FUNC, npp, stream, reason)        \
        (*(FUNC))((npp), (stream), (reason))

#endif


/* NPN_Status */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_StatusUPP;
enum {
    uppNPN_StatusProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(char *)))
};

#define NewNPN_StatusProc(FUNC)     \
        (NPN_StatusUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_StatusProcInfo, GetCurrentArchitecture())
#define CallNPN_StatusProc(FUNC, npp, msg)      \
        (void)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_StatusProcInfo, (npp), (msg))

#else

typedef void (* NP_LOADDS NPN_StatusUPP)(NPP instance, const char* message);
#define NewNPN_StatusProc(FUNC)     \
        ((NPN_StatusUPP) (FUNC))
#define CallNPN_StatusProc(FUNC, npp, msg)      \
        (*(FUNC))((npp), (msg))

#endif


/* NPN_UserAgent */
#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_UserAgentUPP;
enum {
        uppNPN_UserAgentProcInfo = kThinkCStackBased
                | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
                | RESULT_SIZE(SIZE_CODE(sizeof(const char *)))
};

#define NewNPN_UserAgentProc(FUNC)              \
                (NPN_UserAgentUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_UserAgentProcInfo, GetCurrentArchitecture())
#define CallNPN_UserAgentProc(FUNC, ARG1)               \
                (const char*)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_UserAgentProcInfo, (ARG1))

#else

typedef const char* (* NP_LOADDS NPN_UserAgentUPP)(NPP instance);
#define NewNPN_UserAgentProc(FUNC)              \
                ((NPN_UserAgentUPP) (FUNC))
#define CallNPN_UserAgentProc(FUNC, ARG1)               \
                (*(FUNC))((ARG1))

#endif


/* NPN_MemAlloc */
#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_MemAllocUPP;
enum {
    uppNPN_MemAllocProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(uint32_t)))
        | RESULT_SIZE(SIZE_CODE(sizeof(void *)))
};

#define NewNPN_MemAllocProc(FUNC)       \
        (NPN_MemAllocUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_MemAllocProcInfo, GetCurrentArchitecture())
#define CallNPN_MemAllocProc(FUNC, ARG1)        \
        (void*)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_MemAllocProcInfo, (ARG1))

#else

typedef void* (* NP_LOADDS NPN_MemAllocUPP)(uint32_t size);
#define NewNPN_MemAllocProc(FUNC)       \
        ((NPN_MemAllocUPP) (FUNC))
#define CallNPN_MemAllocProc(FUNC, ARG1)        \
        (*(FUNC))((ARG1))

#endif


/* NPN__MemFree */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_MemFreeUPP;
enum {
    uppNPN_MemFreeProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(void *)))
};

#define NewNPN_MemFreeProc(FUNC)        \
        (NPN_MemFreeUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_MemFreeProcInfo, GetCurrentArchitecture())
#define CallNPN_MemFreeProc(FUNC, ARG1)     \
        (void)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_MemFreeProcInfo, (ARG1))

#else

typedef void (* NP_LOADDS NPN_MemFreeUPP)(void* ptr);
#define NewNPN_MemFreeProc(FUNC)        \
        ((NPN_MemFreeUPP) (FUNC))
#define CallNPN_MemFreeProc(FUNC, ARG1)     \
        (*(FUNC))((ARG1))

#endif


/* NPN_MemFlush */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_MemFlushUPP;
enum {
    uppNPN_MemFlushProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(uint32_t)))
        | RESULT_SIZE(SIZE_CODE(sizeof(uint32_t)))
};

#define NewNPN_MemFlushProc(FUNC)       \
        (NPN_MemFlushUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_MemFlushProcInfo, GetCurrentArchitecture())
#define CallNPN_MemFlushProc(FUNC, ARG1)        \
        (uint32_t)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_MemFlushProcInfo, (ARG1))

#else

typedef uint32_t (* NP_LOADDS NPN_MemFlushUPP)(uint32_t size);
#define NewNPN_MemFlushProc(FUNC)       \
        ((NPN_MemFlushUPP) (FUNC))
#define CallNPN_MemFlushProc(FUNC, ARG1)        \
        (*(FUNC))((ARG1))

#endif



/* NPN_ReloadPlugins */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_ReloadPluginsUPP;
enum {
    uppNPN_ReloadPluginsProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPBool)))
        | RESULT_SIZE(SIZE_CODE(0))
};

#define NewNPN_ReloadPluginsProc(FUNC)      \
        (NPN_ReloadPluginsUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_ReloadPluginsProcInfo, GetCurrentArchitecture())
#define CallNPN_ReloadPluginsProc(FUNC, ARG1)       \
        (void)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_ReloadPluginsProcInfo, (ARG1))

#else

typedef void (* NP_LOADDS NPN_ReloadPluginsUPP)(NPBool reloadPages);
#define NewNPN_ReloadPluginsProc(FUNC)      \
        ((NPN_ReloadPluginsUPP) (FUNC))
#define CallNPN_ReloadPluginsProc(FUNC, ARG1)       \
        (*(FUNC))((ARG1))

#endif


/* NPN_GetJavaEnv */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_GetJavaEnvUPP;
enum {
    uppNPN_GetJavaEnvProcInfo = kThinkCStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(JRIEnv*)))
};

#define NewNPN_GetJavaEnvProc(FUNC)     \
        (NPN_GetJavaEnvUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_GetJavaEnvProcInfo, GetCurrentArchitecture())
#define CallNPN_GetJavaEnvProc(FUNC)        \
        (JRIEnv*)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_GetJavaEnvProcInfo)

#else
typedef JRIEnv* (* NP_LOADDS NPN_GetJavaEnvUPP)(void);
#define NewNPN_GetJavaEnvProc(FUNC)     \
        ((NPN_GetJavaEnvUPP) (FUNC))
#define CallNPN_GetJavaEnvProc(FUNC)        \
        (*(FUNC))()

#endif


/* NPN_GetJavaPeer */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_GetJavaPeerUPP;
enum {
    uppNPN_GetJavaPeerProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | RESULT_SIZE(SIZE_CODE(sizeof(jref)))
};

#define NewNPN_GetJavaPeerProc(FUNC)        \
        (NPN_GetJavaPeerUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_GetJavaPeerProcInfo, GetCurrentArchitecture())
#define CallNPN_GetJavaPeerProc(FUNC, ARG1)     \
        (jref)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_GetJavaPeerProcInfo, (ARG1))

#else

typedef jref (* NP_LOADDS NPN_GetJavaPeerUPP)(NPP instance);
#define NewNPN_GetJavaPeerProc(FUNC)        \
        ((NPN_GetJavaPeerUPP) (FUNC))
#define CallNPN_GetJavaPeerProc(FUNC, ARG1)     \
        (*(FUNC))((ARG1))

#endif


/* NPN_InvalidateRect */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_InvalidateRectUPP;
enum {
    uppNPN_InvalidateRectProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPRect *)))
        | RESULT_SIZE(SIZE_CODE(0))
};

#define NewNPN_InvalidateRectProc(FUNC)     \
        (NPN_InvalidateRectUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_InvalidateRectProcInfo, GetCurrentArchitecture())
#define CallNPN_InvalidateRectProc(FUNC, ARG1, ARG2)        \
        (void)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_InvalidateRectProcInfo, (ARG1), (ARG2))

#else

typedef void (* NP_LOADDS NPN_InvalidateRectUPP)(NPP instance, NPRect *rect);
#define NewNPN_InvalidateRectProc(FUNC)     \
        ((NPN_InvalidateRectUPP) (FUNC))
#define CallNPN_InvalidateRectProc(FUNC, ARG1, ARG2)        \
        (*(FUNC))((ARG1), (ARG2))

#endif


/* NPN_InvalidateRegion */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_InvalidateRegionUPP;
enum {
    uppNPN_InvalidateRegionProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPRegion)))
        | RESULT_SIZE(SIZE_CODE(0))
};

#define NewNPN_InvalidateRegionProc(FUNC)       \
        (NPN_InvalidateRegionUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_InvalidateRegionProcInfo, GetCurrentArchitecture())
#define CallNPN_InvalidateRegionProc(FUNC, ARG1, ARG2)      \
        (void)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_InvalidateRegionProcInfo, (ARG1), (ARG2))

#else

typedef void (* NP_LOADDS NPN_InvalidateRegionUPP)(NPP instance, NPRegion region);
#define NewNPN_InvalidateRegionProc(FUNC)       \
        ((NPN_InvalidateRegionUPP) (FUNC))
#define CallNPN_InvalidateRegionProc(FUNC, ARG1, ARG2)      \
        (*(FUNC))((ARG1), (ARG2))

#endif

/* NPN_ForceRedraw */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPN_ForceRedrawUPP;
enum {
    uppNPN_ForceRedrawProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPP)))
        | RESULT_SIZE(SIZE_CODE(sizeof(0)))
};

#define NewNPN_ForceRedrawProc(FUNC)        \
        (NPN_ForceRedrawUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPN_ForceRedrawProcInfo, GetCurrentArchitecture())
#define CallNPN_ForceRedrawProc(FUNC, ARG1)     \
        (jref)CallUniversalProc((UniversalProcPtr)(FUNC), uppNPN_ForceRedrawProcInfo, (ARG1))

#else

typedef void (* NP_LOADDS NPN_ForceRedrawUPP)(NPP instance);
#define NewNPN_ForceRedrawProc(FUNC)        \
        ((NPN_ForceRedrawUPP) (FUNC))
#define CallNPN_ForceRedrawProc(FUNC, ARG1)     \
        (*(FUNC))((ARG1))

#endif


/******************************************************************************************
 * The actual plugin function table definitions
 *******************************************************************************************/

#ifdef XP_MAC
#if PRAGMA_STRUCT_ALIGN
#pragma options align=mac68k
#endif
#endif

typedef struct _NPPluginFuncs {
    uint16_t size;
    uint16_t version;
    NPP_NewUPP newp;
    NPP_DestroyUPP destroy;
    NPP_SetWindowUPP setwindow;
    NPP_NewStreamUPP newstream;
    NPP_DestroyStreamUPP destroystream;
    NPP_StreamAsFileUPP asfile;
    NPP_WriteReadyUPP writeready;
    NPP_WriteUPP write;
    NPP_PrintUPP print;
    NPP_HandleEventUPP event;
    NPP_URLNotifyUPP urlnotify;
    JRIGlobalRef javaClass;
    NPP_GetValueUPP getvalue;
    NPP_SetValueUPP setvalue;
} NPPluginFuncs;

typedef struct _NPNetscapeFuncs {
    uint16_t size;
    uint16_t version;
    NPN_GetURLUPP geturl;
    NPN_PostURLUPP posturl;
    NPN_RequestReadUPP requestread;
    NPN_NewStreamUPP newstream;
    NPN_WriteUPP write;
    NPN_DestroyStreamUPP destroystream;
    NPN_StatusUPP status;
    NPN_UserAgentUPP uagent;
    NPN_MemAllocUPP memalloc;
    NPN_MemFreeUPP memfree;
    NPN_MemFlushUPP memflush;
    NPN_ReloadPluginsUPP reloadplugins;
    NPN_GetJavaEnvUPP getJavaEnv;
    NPN_GetJavaPeerUPP getJavaPeer;
    NPN_GetURLNotifyUPP geturlnotify;
    NPN_PostURLNotifyUPP posturlnotify;
    NPN_GetValueUPP getvalue;
    NPN_SetValueUPP setvalue;
    NPN_InvalidateRectUPP invalidaterect;
    NPN_InvalidateRegionUPP invalidateregion;
    NPN_ForceRedrawUPP forceredraw;
} NPNetscapeFuncs;

#ifdef XP_MAC
#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif
#endif


#ifdef XP_MAC
/******************************************************************************************
 * Mac platform-specific plugin glue stuff
 *******************************************************************************************/

/*
 * Main entry point of the plugin.
 * This routine will be called when the plugin is loaded. The function
 * tables are passed in and the plugin fills in the NPPluginFuncs table
 * and NPPShutdownUPP for Netscape's use.
 */

#if _NPUPP_USE_UPP_

typedef UniversalProcPtr NPP_MainEntryUPP;
enum {
    uppNPP_MainEntryProcInfo = kThinkCStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NPNetscapeFuncs*)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(NPPluginFuncs*)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(NPP_ShutdownUPP*)))
        | RESULT_SIZE(SIZE_CODE(sizeof(NPError)))
};
#define NewNPP_MainEntryProc(FUNC)      \
        (NPP_MainEntryUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNPP_MainEntryProcInfo, GetCurrentArchitecture())
#define CallNPP_MainEntryProc(FUNC,  netscapeFunc, pluginFunc, shutdownUPP)     \
        CallUniversalProc((UniversalProcPtr)(FUNC), (ProcInfoType)uppNPP_MainEntryProcInfo, (netscapeFunc), (pluginFunc), (shutdownUPP))

#else

typedef NPError (* NP_LOADDS NPP_MainEntryUPP)(NPNetscapeFuncs*, NPPluginFuncs*, NPP_ShutdownUPP*);
#define NewNPP_MainEntryProc(FUNC)      \
        ((NPP_MainEntryUPP) (FUNC))
#define CallNPP_MainEntryProc(FUNC,  netscapeFunc, pluginFunc, shutdownUPP)     \
        (*(FUNC))((netscapeFunc), (pluginFunc), (shutdownUPP))

#endif


/*
 * Mac version(s) of NP_GetMIMEDescription(const char *)
 * These can be called to retrieve MIME information from the plugin dynamically
 *
 * Note: For compatibility with Quicktime, BPSupportedMIMEtypes is another way
 *       to get mime info from the plugin only on OSX and may not be supported
 *       in future version--use NP_GetMIMEDescription instead
 */

enum
{
 kBPSupportedMIMETypesStructVers_1    = 1
};

typedef struct _BPSupportedMIMETypes
{
 SInt32    structVersion;      // struct version
 Handle    typeStrings;        // STR# formated handle, allocated by plug-in
 Handle    infoStrings;        // STR# formated handle, allocated by plug-in
} BPSupportedMIMETypes;
OSErr BP_GetSupportedMIMETypes(BPSupportedMIMETypes *mimeInfo, UInt32 flags);

#if _NPUPP_USE_UPP_

#define NP_GETMIMEDESCRIPTION_NAME "NP_GetMIMEDescriptionRD"
typedef UniversalProcPtr NP_GetMIMEDescriptionUPP;
enum {
    uppNP_GetMIMEDescEntryProc = kThinkCStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(const char *)))
};
#define NewNP_GetMIMEDescEntryProc(FUNC)        \
        (NP_GetMIMEDescriptionUPP) NewRoutineDescriptor((ProcPtr)(FUNC), uppNP_GetMIMEDescEntryProc, GetCurrentArchitecture())
#define CallNP_GetMIMEDescEntryProc(FUNC)       \
        (const char *)CallUniversalProc((UniversalProcPtr)(FUNC), (ProcInfoType)uppNP_GetMIMEDescEntryProc)


#else  // !_NPUPP_USE_UPP_

 // NP_GetMIMEDescription
#define NP_GETMIMEDESCRIPTION_NAME "NP_GetMIMEDescription"
typedef const char* (* NP_LOADDS NP_GetMIMEDescriptionUPP)();
#define NewNP_GetMIMEDescEntryProc(FUNC)        \
        ((NP_GetMIMEDescriptionUPP) (FUNC))
#define CallNP_GetMIMEDescEntryProc(FUNC)       \
        (*(FUNC))()
// BP_GetSupportedMIMETypes
typedef OSErr (* NP_LOADDS BP_GetSupportedMIMETypesUPP)(BPSupportedMIMETypes*, UInt32);
#define NewBP_GetSupportedMIMETypesEntryProc(FUNC)      \
        ((BP_GetSupportedMIMETypesUPP) (FUNC))
#define CallBP_GetMIMEDescEntryProc(FUNC,  mimeInfo, flags)     \
        (*(FUNC))((mimeInfo), (flags))

#endif
#endif /* MAC */

#if defined(_WINDOWS)
#define OSCALL WINAPI
#else
#define OSCALL
#endif

#if defined( _WINDOWS )

#ifdef __cplusplus
extern "C" {
#endif

/* plugin meta member functions */

NPError WINAPI NP_GetEntryPoints(NPPluginFuncs* pFuncs);

NPError WINAPI NP_Initialize(NPNetscapeFuncs *aNPNFuncs);

void WINAPI NP_Shutdown();

char*   NP_GetMIMEDescription();

#ifdef __cplusplus
}
#endif

#endif /* _WINDOWS */

#ifdef XP_UNIX

#ifdef __cplusplus
extern "C" {
#endif

/* plugin meta member functions */

char*   NP_GetMIMEDescription(void);
NPError NP_Initialize(NPNetscapeFuncs *aNPNFuncs, NPPluginFuncs *aNPPFuncs);
void NP_Shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* XP_UNIX */

#endif /* _NPUPP_H_ */
