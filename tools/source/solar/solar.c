/*************************************************************************
 *
 *  $RCSfile: solar.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:09 $
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
#include <stdio.h>

#ifdef UNX

#include <unistd.h>
#include <sys/types.h>

#ifdef HPUX
#include <stdlib.h>
#endif

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

#else
#endif

#define printTypeSize(Type,Name)    printf( "sizeof(%s)\t= %d\n", Name, sizeof (Type) )

#define isSignedType(Type)  (((Type)-1) < 0)
#define printTypeSign(Type,Name)    printf( "%s\t= %s %s\n", Name, ( isSignedType(Type) ? "signed" : "unsigned" ), Name )


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
|*  IsStackGrowingDown()
|*
|*  Beschreibung        True, wenn der Stack nach unten waechst
|*
|*  Ersterstellung      EG 26.06.96
|*  Letzte Aenderung
|*
*************************************************************************/
int IsStackGrowingDown_2( int * pI )
{
  int i = 1;
  return ((unsigned long)&i) < (unsigned long)pI;
}

int IsStackGrowingDown()
{
  int i = 1;
  return IsStackGrowingDown_2(&i);
}

/*************************************************************************
|*
|*  IsStackGrowingDown()
|*
|*  Beschreibung        Alignment von char Parametern, die (hoffentlich)
|*                      ueber den Stack uebergeben werden
|*
|*  Ersterstellung      EG 26.06.96
|*  Letzte Aenderung
|*
*************************************************************************/
int GetStackAlignment_3( char*p, long l, int i, short s, char b, char c, ... )
{
  if ( IsStackGrowingDown() )
    return &c - &b;
  else
    return &b - &c;
}

int GetStackAlignment_2( char*p, long l, int i, short s, char b, char c )
{
  if ( IsStackGrowingDown() )
    return &c - &b;
  else
    return &b - &c;
}

int GetStackAlignment()
{
  int nStackAlignment = GetStackAlignment_3(0,1,2,3,4,5);
  if ( nStackAlignment != GetStackAlignment_2(0,1,2,3,4,5) )
    printf( "Pascal calling convention\n" );
  return nStackAlignment;
}


/*************************************************************************
|*
|*  Typdeclarations for memory access test functions
|*
*************************************************************************/
typedef enum { t_char, t_short, t_int, t_long, t_double } Type;
typedef int (*TestFunc)( Type, void* );


#ifdef UNX

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
  /*
  fprintf( stderr, "Signal %d caught\n", sig );
  signal( sig,  SignalHdl );
  /**/
  longjmp( check_env, sig );
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

  if ( !setjmp( check_env ) )
  {
    signal( SIGSEGV,    SignalHdl );
    signal( SIGBUS,     SignalHdl );
    result = func( eT, p );
    signal( SIGSEGV,    SIG_DFL );
    signal( SIGBUS,     SIG_DFL );
  }

  if ( bSignal )
    return -1;
  else
    return 0;
#endif
}

#endif


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
#ifdef DEBUG
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
#ifdef DEBUG
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
  int   bStackGrowsDown;
  int   nStackAlignment;
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
Description_Ctor( struct Description* pThis )
{
  pThis->bBigEndian         = IsBigEndian();
  pThis->bStackGrowsDown    = IsStackGrowingDown();
  pThis->nStackAlignment    = GetStackAlignment();

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
Description_Print( struct Description* pThis, char* name )
{
  int i;
  FILE* f = fopen( name, "w" );
  fprintf( f, "#define __%s\n",
           pThis->bBigEndian ? "BIGENDIAN" : "LITTLEENDIAN" );
  for ( i = 0; i < 3; i++ )
    fprintf( f, "#define __ALIGNMENT%d\t%d\n",
             1 << i+1, pThis->nAlignment[i] );
  fprintf( f, "#define __STACKALIGNMENT wird nicht benutzt\t%d\n", pThis->nStackAlignment );
  fprintf( f, "#define __STACKDIRECTION\t%d\n",
           pThis->bStackGrowsDown ? -1 : 1 );
  fprintf( f, "#define __SIZEOFCHAR\t%d\n", sizeof( char ) );
  fprintf( f, "#define __SIZEOFSHORT\t%d\n", sizeof( short ) );
  fprintf( f, "#define __SIZEOFINT\t%d\n", sizeof( int ) );
  fprintf( f, "#define __SIZEOFLONG\t%d\n", sizeof( long ) );
  fprintf( f, "#define __SIZEOFPOINTER\t%d\n", sizeof( void* ) );
  fprintf( f, "#define __SIZEOFDOUBLE\t%d\n", sizeof( double ) );
  fprintf( f, "#define __IEEEDOUBLE\n" );
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
    printf( "can not read address %p\n", p );

  if ( CheckSetAccess( t_char, p ) )
    printf( "can write address %p\n", p );
  else
    printf( "can not write address %p\n", p );
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
    printf( "Zugriff %s auf %i-Aligned Adresse : ", TypeName( eT ), i / 7 );
    printf( ( CheckGetAccess( eT, (long*)&a[i] ) ? "OK\n" : "ERROR\n" ) );
  }
}

main( int argc, char* argv[] )
{
  printTypeSign( char, "char" );
  printTypeSign( short, "short" );
  printTypeSign( int, "int" );
  printTypeSign( long, "long" );

  printTypeSize( char, "char" );
  printTypeSize( short, "short" );
  printTypeSize( int, "int" );
  printTypeSize( long, "long" );
  printTypeSize( float, "float" );
  printTypeSize( double, "double" );
  printTypeSize( void *, "void *" );

  if ( IsBigEndian() )
    printf( "BIGENDIAN (Sparc, MC680x0, RS6000, IP22, IP32, g3)\n" );
  else
    printf( "LITTLEENDIAN (Intel, VAX, PowerPC)\n" );

  if( IsStackGrowingDown() )
    printf( "Stack waechst nach unten\n" );
  else
    printf( "Stack waechst nach oben\n" );

  printf( "STACKALIGNMENT   : %d\n", GetStackAlignment() );

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
