/*************************************************************************
 *
 *  $RCSfile: typesconfig.c,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 13:29:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* POSIX defines that a program is undefined after a SIG_SEGV.  The
 * code stopped working on Linux Kernel 2.6 so I have moved this back to
 * use FORK.
 * If at a later time the signals work correctly with the Linux Kernel 2.6
 * then this change may be reverted although not strictly posix safe. */
#define USE_FORK_TO_CHECK 1

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

#define I_STDARG
#ifdef I_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#define NO_USE_FORK_TO_CHECK
#ifdef USE_FORK_TO_CHECK
#include <sys/wait.h>
#else
#include <signal.h>
#include <setjmp.h>
#endif

#define printTypeSize(Type,Name)    printf( "sizeof(%s)\t\t= %d\n", Name, sizeof (Type) )

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
#ifdef I_STDARG
void PrintArgs( int p, ... )
#else
void PrintArgs( p, va_alist )
int p;
va_dcl
#endif
{
    int value;
    va_list ap;

#ifdef I_STDARG
    va_start( ap, p );
#else
    va_start( ap );
#endif

    printf( "value = %d", p );

    while ( ( value = va_arg(ap, int) ) != 0 )
      printf( " %d", value );

    printf( "\n" );
    va_end(ap);
}

#ifndef USE_FORK_TO_CHECK
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
static jmp_buf check_env;
static int bSignal;
void SignalHdl( int sig )
{
  bSignal = 1;

  fprintf( stderr, "Signal %d caught\n", sig );
  signal( SIGSEGV,  SIG_DFL );
  signal( SIGBUS,   SIG_DFL );
  siglongjmp( check_env, sig );
}
#endif

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
#ifdef USE_FORK_TO_CHECK
  pid_t nChild = fork();
  if ( nChild )
  {
    int exitVal;
    wait( &exitVal );
    if ( exitVal & 0xff )
      return -1;
    else
      return exitVal >> 8;
  }
  else
  {
    exit( func( eT, p ) );
  }
#else
  int result;

  bSignal = 0;

  if ( !sigsetjmp( check_env, 1 ) )
  {
    signal( SIGSEGV,    SignalHdl );
    signal( SIGBUS, SignalHdl );
    result = func( eT, p );
    signal( SIGSEGV,    SIG_DFL );
    signal( SIGBUS, SIG_DFL );
  }

  if ( bSignal )
    return -1;
  else
    return 0;
#endif
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
int GetAtAddress( Type eT, void* p )
{
  switch ( eT )
  {
  case t_char:      return *((char*)p);
  case t_short:     return *((short*)p);
  case t_int:       return *((int*)p);
  case t_long:      return *((long*)p);
  case t_double:    return *((double*)p);
  }
  abort();
}

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
  fprintf( f, "#define SAL_TYPES_SIZEOFSHORT\t%d\n", sizeof( short ) );
  fprintf( f, "#define SAL_TYPES_SIZEOFINT\t%d\n", sizeof( int ) );
  fprintf( f, "#define SAL_TYPES_SIZEOFLONG\t%d\n", sizeof( long ) );
  fprintf( f, "#define SAL_TYPES_SIZEOFLONGLONG\t%d\n", sizeof( long long ) );
  fprintf( f, "#define SAL_TYPES_SIZEOFPOINTER\t%d\n", sizeof( void* ) );

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
