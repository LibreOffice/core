// stdafx1.cpp : source file that includes just the standard includes
//  stdafx1.pch will be the pre-compiled header
//  stdafx1.obj will contain the pre-compiled type information

#include "stdafx2.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#undef _DEBUG
#endif
#include <atlimpl.cpp>
