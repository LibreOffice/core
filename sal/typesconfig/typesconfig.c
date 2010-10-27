/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

#include <stdarg.h>

#include <signal.h>
#include <setjmp.h>

#define printTypeSize(Type,Name)    printf( "sizeof(%s)\t\t= %d\n", Name, (int) sizeof (Type) )

#define isSignedType(Type)  (((Type)-1) < 0)
#define printTypeSign(Type,Name)    printf( "%s\t\t= %s %s\n", Name, ( isSignedType(Type) ? "signed" : "unsigned" ), Name )


/*************************************************************************
|*
|*  IsBigEndian()
|*
|*  Beschreibung        True, wenn CPU BigEndian ist
|*
|*  Ersterstellung      EG 26.06.96
|*  Letzte Aenderung
|*
*************************************************************************/
int IsBigEndian()
{
  long l = 1;
  return ! *(char*)&l;
}

/*************************************************************************
|*
|*  Typdeclarations for memory access test functions
|*
*************************************************************************/
typedef enum { t_char, t_short, t_int, t_long, t_double } Type;
typedef int (*TestFunc)( Type, void* );


/*************************************************************************
|*
|*  PrintArgs()
|*
|*  Beschreibung        Testfunktion fuer variable Parameter
|*
|*  Ersterstellung      EG 26.06.96
|*  Letzte Aenderung
|*
*************************************************************************/
void PrintArgs( int p, ... )
{
    int value;
    va_list ap;

    va_start( ap, p );

    printf( "value = %d", p );

    while ( ( value = va_arg(ap, int) ) != 0 )
      printf( " %d", value );

    printf( "\n" );
    va_end(ap);
}

/*************************************************************************
|*
|*  SignalHdl()
|*
|*  Beschreibung        faengt SIGBUS und SIGSEGV in check() ab
|*
|*  Ersterstellung      EG 26.06.96
|*  Letzte Aenderung
|*
*************************************************************************/

static sigjmp_buf jmpbuf;
static volatile sig_atomic_t hit;

void SignalHdl( int sig )
{
    (void) sig; // ignored
    hit = 1;
    siglongjmp(jmpbuf, 0);
}

/*************************************************************************
|*
|*  check()
|*
|*  Beschreibung        Testet MemoryZugriff (read/write)
|*
|*  Ersterstellung      EG 26.06.96
|*  Letzte Aenderung
|*
*************************************************************************/
int check( TestFunc func, Type eT, void* p )
{
    hit = 0;
    if (sigsetjmp(jmpbuf, 1) == 0) {
        struct sigaction sa;
        sa.sa_handler = SignalHdl;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        if (sigaction(SIGBUS, &sa, NULL) != 0 ||
            sigaction(SIGSEGV, &sa, NULL) != 0)
        {
            abort();
        }
        func(eT, p);
        sa.sa_handler = SIG_DFL;
        if (sigaction(SIGBUS, &sa, NULL) != 0 ||
            sigaction(SIGSEGV, &sa, NULL) != 0)
        {
            abort();
        }
    }
    return hit ? -1 : 0;
}

/*************************************************************************
|*
|*  GetAtAddress()
|*
|*  Beschreibung        memory read access
|*
|*  Ersterstellung      EG 26.06.96
|*  Letzte Aenderung
|*
*************************************************************************/
#if defined(IA64) || defined(ARM32) || defined(HPPA) || defined(AXP)

int forceerror()
{
#if defined(ARM32)
// workaround for qemu-user
    hit = 1;
#else
    raise (SIGBUS);
#endif
    return 1;
}

int GetAtAddress( Type eT, void* p )
{
  switch ( eT )
  {
  case t_char:      return *((char*)p);
  case t_short:     if ((long)p % sizeof(short)) return forceerror(); else return *((short*)p);
  case t_int:       if ((long)p % sizeof(int)) return forceerror(); else return *((int*)p);
  case t_long:      if ((long)p % sizeof(long)) return forceerror(); else return *((long*)p);
  case t_double:    if ((long)p % sizeof(double)) return forceerror(); else return *((double*)p);
  }
  abort();
}

#else
static int dummy(void* unused);

int GetAtAddress( Type eT, void* p )
{
  switch ( eT )
  {
  case t_char: { char x = *(char*)p; return dummy(&x); }
  case t_short: { short x = *(short*)p; return dummy(&x); }
  case t_int: { int x = *(int*)p; return dummy(&x); }
  case t_long: { long x = *(long*)p; return dummy(&x); }
  case t_double: { double x = *(double*)p; return dummy(&x); }
  }
  abort();
}

int dummy(void* unused)
{
    (void)unused;
    return 0;
}

#endif
/*************************************************************************
|*
|*  SetAtAddress()
|*
|*  Beschreibung        memory write access
|*
|*  Ersterstellung      EG 26.06.96
|*  Letzte Aenderung
|*
*************************************************************************/
int SetAtAddress( Type eT, void* p )
{
  switch ( eT )
  {
  case t_char:      return *((char*)p)  = 0;
  case t_short:     return *((short*)p) = 0;
  case t_int:       return *((int*)p)   = 0;
  case t_long:      return *((long*)p)  = 0;
  case t_double:    return *((double*)p)= 0;
  }
  abort();
}

char* TypeName( Type eT )
{
  switch ( eT )
  {
  case t_char:      return "char";
  case t_short:     return "short";
  case t_int:       return "int";
  case t_long:      return "long";
  case t_double:    return "double";
  }
  abort();
}

/*************************************************************************
|*
|*  Check(Get|Set)Access()
|*
|*  Beschreibung        Testet MemoryZugriff (read/write)
|*                      Zugriffsverletzungen werden abgefangen
|*
|*  Ersterstellung      EG 26.06.96
|*  Letzte Aenderung
|*
*************************************************************************/
int CheckGetAccess( Type eT, void* p )
{
  int b;
  b = -1 != check( (TestFunc)GetAtAddress, eT, p );
#if OSL_DEBUG_LEVEL > 1
  fprintf( stderr,
           "%s read %s at %p\n",
           (b? "can" : "can not" ), TypeName(eT), p );
#endif
  return b;
}
int CheckSetAccess( Type eT, void* p )
{
  int b;

  b = -1 != check( (TestFunc)SetAtAddress, eT, p );
#if OSL_DEBUG_LEVEL > 1
  fprintf( stderr,
           "%s write %s at %p\n",
           (b? "can" : "can not" ), TypeName(eT), p );
#endif
  return b;
}

/*************************************************************************
|*
|*  GetAlignment()
|*
|*  Beschreibung        Bestimmt das Alignment verschiedener Typen
|*
|*  Ersterstellung      EG 26.06.96
|*  Letzte Aenderung
|*
*************************************************************************/
int GetAlignment( Type eT )
{
  char  a[ 16*8 ];
  long  p = (long)(void*)a;
  int   i;

  /* clear a[...] to set legal value for double access */
  for ( i = 0; i < 16*8; i++ )
    a[i] = 0;

  p = ( p + 0xF ) & ~0xF;
  for ( i = 1; i < 16; i++ )
    if ( CheckGetAccess( eT, (void*)(p+i) ) )
      return i;
  return 0;
}

/*************************************************************************
|*
|*  struct Description
|*
|*  Beschreibung        Beschreibt die Parameter der Architektur
|*
|*  Ersterstellung      EG 26.06.96
|*  Letzte Aenderung
|*
*************************************************************************/
struct Description
{
  int   bBigEndian;
  int   nAlignment[3];  /* 2,4,8 */
};

/*************************************************************************
|*
|*  Description_Ctor()
|*
|*  Beschreibung        Bestimmt die Parameter der Architektur
|*
|*  Ersterstellung      EG 26.06.96
|*  Letzte Aenderung
|*
*************************************************************************/
void Description_Ctor( struct Description* pThis )
{
  pThis->bBigEndian         = IsBigEndian();

  if ( sizeof(short) != 2 )
    abort();
  pThis->nAlignment[0] = GetAlignment( t_short );
  if ( sizeof(int) != 4 )
    abort();
  pThis->nAlignment[1] = GetAlignment( t_int );

  if      ( sizeof(long) == 8 )
    pThis->nAlignment[2] = GetAlignment( t_long );
  else if ( sizeof(double) == 8 )
    pThis->nAlignment[2] = GetAlignment( t_double );
  else
    abort();
}

/*************************************************************************
|*
|*  Description_Print()
|*
|*  Beschreibung        Schreibt die Parameter der Architektur als Header
|*
|*  Ersterstellung      EG 26.06.96
|*  Letzte Aenderung
|*
*************************************************************************/
void Description_Print( struct Description* pThis, char* name )
{
  int i;
  FILE* f = fopen( name, "w" );
  if( ! f ) {
      fprintf( stderr, "Unable to open file %s: %s\n", name, strerror( errno ) );
      exit( 99 );
  }
  fprintf( f, "/* This file is autogenerated from the 'typesconfig' program\n * in the sal module\n */\n\n" );

/* Disabled for now in preference to big/little endian defines in <osl/endian.h>  fa (2004-03-15) */
/*  fprintf( f, "#define SAL_TYPES_%s\n", pThis->bBigEndian ? "BIGENDIAN" : "LITTLEENDIAN" ); */

  for ( i = 0; i < 3; i++ )
    fprintf( f, "#define SAL_TYPES_ALIGNMENT%d\t%d\n",  1 << (i+1), pThis->nAlignment[i] );
  fprintf( f, "#define SAL_TYPES_SIZEOFSHORT\t%d\n", (int) sizeof( short ) );
  fprintf( f, "#define SAL_TYPES_SIZEOFINT\t%d\n", (int) sizeof( int ) );
  fprintf( f, "#define SAL_TYPES_SIZEOFLONG\t%d\n", (int) sizeof( long ) );
  fprintf( f, "#define SAL_TYPES_SIZEOFLONGLONG\t%d\n", (int) sizeof( long long ) );
  fprintf( f, "#define SAL_TYPES_SIZEOFPOINTER\t%d\n", (int) sizeof( void* ) );

/* Disabled for now, becuase OOo code assumes sizeof(double) == 8 and this is not
 * likely to change any time soon.  fa (2004-03-15)
 */
/*  fprintf( f, "#define SAL_TYPES_SIZEOFDOUBLE\t%d\n", sizeof( double ) );*/

  fclose( f );
}

/*************************************************************************
|*
|*  InfoMemoryAccess()
|*
|*  Beschreibung        Informeller Bytezugriffstest
|*
|*  Ersterstellung      EG 26.06.96
|*  Letzte Aenderung
|*
*************************************************************************/
void InfoMemoryAccess( char* p )
{
  if ( CheckGetAccess( t_char, p ) )
    printf( "can read address %p\n", p );
  else
    printf( "cannot read address %p\n", p );

  if ( CheckSetAccess( t_char, p ) )
    printf( "can write address %p\n", p );
  else
    printf( "cannot write address %p\n", p );
}

/*************************************************************************
|*
|*  InfoMemoryTypeAccess()
|*
|*  Beschreibung        Informeller Zugriffstest verschiedener Typen
|*
|*  Ersterstellung      EG 15.08.96
|*  Letzte Aenderung
|*
*************************************************************************/
void InfoMemoryTypeAccess( Type eT )
{
  char  a[64];
  int   i;

  /* clear a[...] to set legal value for double access */
  for ( i = 0; i < 64; i++ )
    a[i] = 0;

  for ( i = 56; i >= 7; i >>= 1 )
  {
    if ( CheckGetAccess(eT, (long*)&a[i]) )
        printf( "Access %s on %i-Aligned Address : OK\n", TypeName(eT), i / 7 );
    else
        printf( "Access %s on %i-Aligned Address : ERROR\n", TypeName(eT), i / 7 );
  }
}
/************************************************************************
 *
 *  Use C code to determine the characteristics of the building platform.
 *
 ************************************************************************/
int main( int argc, char* argv[] )
{
  printTypeSign( char, "char" );
  printTypeSign( short, "short" );
  printTypeSign( int, "int" );
  printTypeSign( long, "long" );
  printTypeSign( long long, "long long" );

  printTypeSize( short, "short" );
  printTypeSize( int, "int" );
  printTypeSize( long, "long" );
  printTypeSize( long long, "long long" );
  printTypeSize( float, "float" );
  printTypeSize( double, "double" );
  printTypeSize( void *, "void *" );

  if ( IsBigEndian() )
    printf( "BIGENDIAN (Sparc, RS6000, IP22, IP32, PowerPC(BE))\n" );
  else
    printf( "LITTLEENDIAN (Intel, x86-64, PowerPC(LE))\n" );

  /* PrintArgs( 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 ); */

  if ( argc > 1 )
  {
    struct Description description;
    Description_Ctor( &description );
    Description_Print( &description, argv[1] );
  }
  {
    char* p = NULL;
    InfoMemoryAccess( p );
    p = (char*)&p;
    InfoMemoryAccess( p );
    InfoMemoryTypeAccess( t_short );
    InfoMemoryTypeAccess( t_int );
    InfoMemoryTypeAccess( t_long );
    InfoMemoryTypeAccess( t_double );
  }

  exit( 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
