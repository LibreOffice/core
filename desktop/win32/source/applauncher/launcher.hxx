#pragma once
#ifndef __cplusplus
#error Need C++ to compile
#endif

#ifndef _INC_TCHAR
#	ifdef UNICODE
#		define _UNICODE
#	endif
#	include <tchar.h>
#endif

#ifdef UNICODE
#	define GetArgv( pArgc )			CommandLineToArgvW( GetCommandLine(), pArgc )
#else
#	define GetArgv( pArgc )			(*pArgc = __argc, __argv)
#endif

#define OFFICE_IMAGE_NAME	_T("soffice")

extern _TCHAR APPLICATION_SWITCH[];
