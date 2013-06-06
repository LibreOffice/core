/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/* From mingw-w64 excpt.h */

#ifndef _SEHANDLER_HXX
#define _SEHANDLER_HXX

#if !defined( __MINGW32__ ) || !defined ( _WIN64 )
#error This file should be included only in a 64-bit MinGW compilation
#endif

#ifdef __try
#undef __try
#endif

#if 0
#define __try \
  __asm__ __volatile__ ("\t.l_startw:\n" \
  );
#else
#define __try \
  if (true)
#endif

#ifdef __except
#undef __except
#endif

#if 0
#define __except(exvalue) \
  __asm__ __volatile__ ("\tnop\n" \
  "\t.seh_handler __C_specific_handler, @except\n" \
  "\t.seh_handlerdata\n" \
  "\t.long 1\n" \
  "\t.rva .l_startw, .l_endw, " __MINGW64_STRINGIFY(exvalue) " ,.l_endw\n" \
  "\t.text\n" \
  "\t.l_endw: nop\n" \
  );
#else
#define __except(exvalue) \
  if (false)
#endif


#endif // _SEHANDLER_HXX

