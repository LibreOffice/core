/*************************************************************************
 *
 *  $RCSfile: pythontest.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:56 $
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
#include <stardiv/uno/repos/implementationregistration.hxx>
#include <stardiv/uno/script/script.hxx>
#include <stardiv/uno/beans/exactname.hxx>

#include <rtl/ustring.hxx>
#include <vos/dynload.hxx>
#include <vos/diagnose.hxx>
#include <usr/services.hxx>
#include <vcl/svapp.hxx>
#include <usr/ustring.hxx>
#include <usr/weak.hxx>
#include <tools/string.hxx>
#include <vos/conditn.hxx>
#include <assert.h>

#ifdef _USE_NAMESPACE
using namespace rtl;
using namespace vos;
using namespace usr;
#endif

#define PCHAR_TO_USTRING(x) StringToOUString(String(x),CHARSET_SYSTEM)



class NullEngineListenerRef : public XEngineListenerRef
{
    virtual void interrupt(const InterruptEngineEvent& Evt) THROWS( (UsrSystemException) ) {}
    virtual void running(const EventObject& Evt) THROWS( (UsrSystemException) ) {}
    virtual void finished(const FinishEngineEvent& Evt) THROWS( (UsrSystemException) ) {}
};

#define USTRING_TO_PCHAR(x) OUStringToString(x , CHARSET_DONTKNOW ).GetCharStr()

class CmdDebugger :
    public XEngineListener,
    public OWeakObject
{
public:

    CmdDebugger()
    {
        m_pDebuggingRef = 0;
        m_pEngineRef = 0;
        m_bIsTerminating = FALSE;
        m_bIsRunning = FALSE;
    }


    CmdDebugger( XDebuggingRef *p, XEngineRef *pEngine , XInvokationRef *pInvokation)
    {
        attach( p , pEngine , pInvokation );
    }

    ~CmdDebugger()
    {
        if( m_pDebuggingRef ) {
            detach();
        }
    }

    BOOL                queryInterface( Uik aUik, XInterfaceRef & rOut );
    void                acquire()                        { OWeakObject::acquire(); }
    void                release()                        { OWeakObject::release(); }
    void*               getImplementation(Reflection *p) { return OWeakObject::getImplementation(p); }


    void attach( XDebuggingRef *p , XEngineRef *pEngine , XInvokationRef *pInvokation )
    {
        m_pDebuggingRef = p;
        m_pEngineRef = pEngine;
        m_pInvokationRef = pInvokation;
        m_bIsRunning = FALSE;
        m_bIsTerminating = FALSE;
    }

    void detach( );


    virtual void disposing( const EventObject &o )
    {
        if( m_pDebuggingRef ) {
            detach();
        }
    }
    virtual void interrupt(const InterruptEngineEvent& Evt) THROWS( (UsrSystemException) )
    {
        if( m_pDebuggingRef && ! m_bIsTerminating ) {
               (*m_pDebuggingRef)->stop();
               fprintf( stderr, "%s\n" , USTRING_TO_PCHAR(Evt.ErrorMessage ) );
            fprintf( stderr, "%s.%s (%d)\n",  USTRING_TO_PCHAR(Evt.SourceCode),
                                              USTRING_TO_PCHAR(Evt.Name ),
                                              Evt.StartLine );
               m_aDebugCondition.set();
            m_bIsRunning = TRUE;
           }
    }

    virtual void running(const EventObject& Evt) THROWS( (UsrSystemException) )
       {
        if( m_pDebuggingRef && ! m_bIsTerminating ) {
               (*m_pDebuggingRef)->stop();

            m_aDebugCondition.set();
            m_bIsRunning = TRUE;
            fprintf( stderr, "%s\n" , "Script starts\n" );
        }
    }

    virtual void finished(const FinishEngineEvent& Evt) THROWS( (UsrSystemException) )
    {
        if( m_pDebuggingRef && ! m_bIsTerminating  ) {
            m_aDebugCondition.set();
            m_bIsRunning = FALSE;
            fprintf( stderr , "%s\n", USTRING_TO_PCHAR( Evt.ErrorMessage ) );
        }
    }

    void dumpIntrospectionToStream( const XIntrospectionAccessRef &, FILE *f );
    void dumpVarToStream( const char *pcName, const UsrAny &any, FILE *f );


    void cmdLine();
protected:

    OCondition m_aDebugCondition;
    XDebuggingRef *m_pDebuggingRef;
    XEngineRef  *m_pEngineRef;
    XInvokationRef *m_pInvokationRef;
    int m_bIsRunning;
    int m_bIsTerminating;       // The listeners ignore everything when set
};



void CmdDebugger::cmdLine()
{
        char pcLine[80];
        fprintf( stderr, "entering debugger\n" );
        while( TRUE ) {

            m_aDebugCondition.wait();

            fprintf( stderr , "(debug %d) : " , m_bIsRunning );
            fflush( stderr);
            fgets( pcLine ,  79 , stdin );

            if( strlen( pcLine) ) pcLine[strlen(pcLine)-1] =0;
            String sLine( pcLine );

            if( ! strcmp( pcLine , "g" ) ) {
                if( m_bIsRunning ) {
                    m_aDebugCondition.reset();
                    (*m_pDebuggingRef)->doContinue();
                }
                else fprintf( stderr,"no script running !\n" );
            }
            else if( ! strcmp( pcLine , "s" ) ) {
                if( m_bIsRunning ) {
                    m_aDebugCondition.reset();
                    (*m_pDebuggingRef)->stepOver();
                }
                else fprintf(stderr, "no script running !\n" );
            }
            else if( ! strcmp( pcLine , "so" ) ) {
                if( m_bIsRunning ) {
                    m_aDebugCondition.reset();
                    (*m_pDebuggingRef)->stepOut();
                }
                else fprintf(stderr, "no script running !\n" );
            }
            else if( ! strcmp( pcLine , "si" ) ) {
                if( m_bIsRunning ) {
                    m_aDebugCondition.reset();
                    (*m_pDebuggingRef)->stepIn();
                }
                else fprintf(stderr, "no script running !\n" );
            }
            else if( ! strncmp( pcLine , "sbp" , 3 ) ){
                if( m_bIsRunning ) {
                    (*m_pDebuggingRef)->setBreakPoint(  UString( L"<string>" ),
                                                        atoi(&pcLine[3]) , TRUE );
                }
            }
            else if( ! strncmp( pcLine , "rbp" , 3 ) ){
                if( m_bIsRunning ) {
                    (*m_pDebuggingRef)->setBreakPoint(  UString( L"<string>" ),
                                                        atoi(&pcLine[3]) , FALSE );
                }
            }
            else if( ! strncmp( pcLine , "dv " , 3 ) ) {
                if( m_bIsRunning ) {
                    int nCallstack = 0;
                    if( sLine.GetQuotedTokenCount( String("''"),' ' ) == 3 ) {
                        nCallstack = atoi( sLine.GetQuotedToken( 3 , String("''"), ' ' ).GetCharStr() );
                    }

                    UString str = (*m_pDebuggingRef)->dumpVariable(
                                                    PCHAR_TO_USTRING( &pcLine[3]),nCallstack);
                    fprintf( stderr, "%s\n" , USTRING_TO_PCHAR( str ) );
                }
            }
            else if( ! strncmp( pcLine , "sv " , 3 ) ) {
                int nCallstack = 0;
                if( sLine.GetQuotedTokenCount( String("''"),' ' ) == 3 ) {
                    nCallstack = atoi( sLine.GetQuotedToken( 3 , String("''"), ' ' ).GetCharStr() );
                }
                (*m_pDebuggingRef)->setVariable(
                        StringToOUString( sLine.GetQuotedToken( 1 , String("''"), ' ' ), CHARSET_SYSTEM ),
                        StringToOUString( sLine.GetQuotedToken( 2 , String("''"), ' ' ), CHARSET_SYSTEM ),
                        nCallstack );

            }
            else if( ! strncmp( pcLine , "ci" ,2 ) ) {
                if( m_bIsRunning ) {
                    UString *aUString ;
                    ContextInformation ci = (*m_pDebuggingRef)->getContextInformation(atoi(&pcLine[2]));
                    int i,iMax;

                    fprintf( stderr, "File %s (%d)\n", USTRING_TO_PCHAR(ci.Name),
                                                       ci.StartLine );
                    fprintf( stderr, "Available variables : \n" );
                    aUString = ci.LocalVariableNames.getArray();
                    iMax = ci.LocalVariableNames.getLen();

                    for( i = 0 ; i < iMax ; i++ ) {
                        fprintf( stderr, "      %s\n" , USTRING_TO_PCHAR( aUString[i]) );
                    }
                }
            }
            else if ( !strcmp( pcLine , "d" ) ) {
                if( m_bIsRunning ) {
                    UString * aUString ;
                    Sequence<UString> seq =  (*m_pDebuggingRef)->getStackTrace();

                    aUString = seq.getArray();
                    int iMax = seq.getLen();
                    for( int i = 0; i < iMax ; i++ ) {
                        fprintf( stderr , "%s\n" , USTRING_TO_PCHAR( aUString[i] ) );
                    }
                }
            }
            else if( !strcmp( pcLine , "c" ) ) {
                if( m_bIsRunning ) {
                    (*m_pEngineRef)->cancel();
                    m_aDebugCondition.reset();
                }
                else fprintf( stderr,"no script running !\n" );
            }
            else if( !strcmp( pcLine , "q" ) ) {
                if( m_bIsRunning ) {
                    m_aDebugCondition.reset();
                    (*m_pEngineRef)->cancel();
                }
                else {
                    m_bIsTerminating = TRUE;
                    fprintf(stderr,  "Debugger terminates\n" );
                    break;
                }
            }
            else if( ! strcmp( pcLine , "id" ) ) {

                XIntrospectionAccessRef ref = (*m_pInvokationRef)->getIntrospection();

                dumpIntrospectionToStream( ref , stderr );


            }
            else if( ! strncmp( pcLine , "idv" , 3) ) {
                TRY {
                    UsrAny any = (*m_pInvokationRef)->getValue( PCHAR_TO_USTRING( &(pcLine[4]) ) );
                    dumpVarToStream( &(pcLine[4]) , any , stderr );
                }
                CATCH(UnknownPropertyException,e) {
                    fprintf( stderr, "UnknownPropertyException\n" );
                }
                CATCH(IllegalArgumentException,e) {
                    fprintf( stderr, "IllegalArgumentException\n" );
                }
            }
            else if( !strcmp( pcLine , "t" ) ) {
            }
            else if( !strcmp( pcLine , "h" ) ) {
                fprintf( stderr ,  "\nvalid commands :\n"
                                   "Go                 : g\n"
                                   "StepOver           : s\n"
                                   "StepIn             : si\n"
                                   "StepOut            : so\n"
                                   "Set BreakPoint     : sbp Line [ModuleName]\n"
                                   "Remove BreakPoint  : rbp [Line] [ModuleName]\n"
                                   "via XDebugging Interface :\n"
                                   "    dump Variable      : dv varname [CallStack]\n"
                                   "    set Variable       : sv varname value [CallStack]\n"
                                   "globals via XInvokation Interface :\n"
                                   "    dump Global vars   : id\n"
                                   "    dump Variable      : idv varname\n"
                                   "    set Variable       : isv varname value\n"
                                   "ContextInformation : ci\n"
                                   "Dump callstack     : d\n"
                                   "Cancel             : c (stops actual script)\n"
                                   "Quit               : q (exits debugger)\n"
                                   );
            }
            else if( ! strlen( pcLine ) ) {
            }
            else {
                fprintf( stderr , "unknown command %s\n" , pcLine );
            }
        }
}

void CmdDebugger::dumpIntrospectionToStream( const XIntrospectionAccessRef &ref, FILE *f )
{
    int i,iMax;
    fprintf( stderr, "Callable Attributes (Methods) :\n" );
    Sequence<XIdlMethodRef> seq = ref->getMethods( 0 );
    iMax = seq.getLen();
    XIdlMethodRef *aRef = seq.getArray();
    for( i = 0; i < iMax ; i++ ) {
        fprintf( f, "  %s\n" , USTRING_TO_PCHAR( aRef[i]->getName( ) ) );
    }

    fprintf( stderr, "Other attributes\n"  );
    Sequence<Property> seqProp = ref->getProperties( 0 );
    iMax = seqProp.getLen();
    Property *aProp = seqProp.getArray();
    for( i = 0; i < iMax ; i ++ ) {
        fprintf( f, "  %s %s\n" ,   USTRING_TO_PCHAR( aProp[i].Type->getName() ),
                                        USTRING_TO_PCHAR( aProp[i].Name ) );
    }

}

void CmdDebugger::dumpVarToStream( const char *pc , const UsrAny &aValue, FILE *f )
{
    TypeClass type = aValue.getReflection()->getTypeClass();

    if( TypeClass_INT == type ) {
        fprintf( f, "INT32 %s : %d\n" , pc , aValue.getINT32() );
    }
    else if( TypeClass_ENUM == type ) {
        fprintf( f, "ENUM %s : %d\n", pc ,  aValue.getEnumAsINT32() );
    }
    else if( TypeClass_STRING == type ) {
        fprintf( f, "STRING %s : %s\n" , pc ,  USTRING_TO_PCHAR( aValue.getString())     );
    }
    else if( TypeClass_BOOLEAN == type ) {
        fprintf( f, "BOOL %s : %d\n", pc , aValue.getBOOL() );
    }
    else if( TypeClass_CHAR == type  ) {
        fprintf( f, "char %s : %d\n", pc , ( INT32) aValue.getChar() );
    }
    else if( TypeClass_SHORT == type ) {
        fprintf( f, "INT16 %s : %d\n", pc , (INT32) aValue.getINT16());
    }
    else if( TypeClass_LONG == type ) {
        fprintf( f, "LONG %s : %d\n", pc , (INT32) aValue.getINT32());
    }
    else if( TypeClass_UNSIGNED_SHORT == type ) {
        fprintf( f, "UINT16 %s : %d\n", pc , (INT32) aValue.getUINT16() );
    }
    else if( TypeClass_UNSIGNED_BYTE == type ) {
        fprintf( f, "Byte %s : %d\n", pc ,  (INT32) aValue.getBYTE() );
    }
    else if( TypeClass_UNSIGNED_INT == type ) {
        fprintf( f, "UINT32 %s : %d\n", pc , aValue.getUINT32() );
    }
    else if( TypeClass_FLOAT == type ) {
        fprintf( f, "float %s : %f\n" , pc , aValue.getFloat() );
    }
    else if( TypeClass_DOUBLE == type ) {
        fprintf( f, "double %s : %f\n" , pc , aValue.getDouble() );
    }
    else if( TypeClass_VOID == type ) {
        fprintf( f, "void %s :\n" , pc );
    }
    else if( TypeClass_INTERFACE == type ) {
        // Check, what has been put in
        if( aValue.getReflection() == XPropertySet_getReflection() ) {
            // XPropertySet !
            XPropertySetRef *pRef = ( XPropertySetRef * ) aValue.get();
            XPropertySetInfoRef refInfo = (*pRef)->getPropertySetInfo();
            Sequence< Property > seq = refInfo->getProperties();
            int i,iMax = seq.getLen();

            Property *pArray;
            pArray = seq.getArray();
            fprintf( stderr, "Property List :\n" );
            for( i = 0; i < iMax ; i ++ ) {
                fprintf( f, "%s\t %s\n" , USTRING_TO_PCHAR(pArray[i].Type->getName()),
                                                 USTRING_TO_PCHAR( pArray[i].Name ) );
            }
        }
        else if( aValue.getReflection() == XInvokation_getReflection() ) {
            XInvokationRef *pRef = ( XInvokationRef * ) aValue.get();
            XIntrospectionAccessRef refIntro = (*pRef)->getIntrospection();

            dumpIntrospectionToStream( refIntro, stderr );
        }
    }
    else if( TypeClass_SEQUENCE == type ) {
        fprintf( f , "%s Sequence \n" , pc  );
        String s( "   " );
        s += pc;
        SequenceReflection *pSeqRefl = ( SequenceReflection * ) aValue.getReflection();

        int i,iMax = pSeqRefl->getLen( aValue );

        for( i = 0 ; i < iMax ; i ++ ) {
            dumpVarToStream( s.GetCharStr() , pSeqRefl->get( aValue , i ) , stderr );
        }
    }
    else {
        fprintf( f, "%s : unknown %d\n" , pc , type  );
    }

}

void CmdDebugger::detach()
{
    assert( m_pDebuggingRef );

       m_bIsRunning = FALSE;
       m_pDebuggingRef = 0;
    m_pEngineRef = 0;
    m_pInvokationRef = 0;
}

// Methoden von XInterface
BOOL CmdDebugger::queryInterface( Uik aUik, XInterfaceRef & rOut )
{
    if( aUik == XEngineListener::getSmartUik() )
        rOut = (XEngineListener*)this;
    else
        return OWeakObject::queryInterface( aUik, rOut );
    return TRUE;
}






/*
 * main.
 */
int __LOADONCALLAPI main (int argc, char **argv)
{
    XMultiServiceFactoryRef xSMgr = createRegistryServiceManager();
    registerUsrServices( xSMgr );
    setProcessServiceManager( xSMgr );

    XInterfaceRef x = xSMgr->createInstance( L"stardiv.uno.repos.ImplementationRegistration" );
    XImplementationRegistrationRef xReg( x, USR_QUERY );
    sal_Char szBuf[1024];

    ORealDynamicLoader::computeModuleName( "pythonengine", szBuf, 1024 );
    UString aDllName( StringToOUString( szBuf, CHARSET_SYSTEM ) );
    xReg->registerImplementation( L"stardiv.loader.SharedLibrary", aDllName, XSimpleRegistryRef() );

    ORealDynamicLoader::computeModuleName( "aps", szBuf, 1024 );
    aDllName = UString( StringToOUString( szBuf, CHARSET_SYSTEM ) );
    xReg->registerImplementation( L"stardiv.loader.SharedLibrary", aDllName, XSimpleRegistryRef() );

    XInterfaceRef y = xSMgr->createInstance( L"stardiv.script.Python" );
    XEngineRef yEngine( y, USR_QUERY );

    x = xSMgr->createInstance( L"stardiv.script.Python" );
    XEngineRef xEngine( x, USR_QUERY );


    UString Script;

    Sequence<UsrAny> args(3);
    UsrAny *pArray = args.getArray();
    pArray[0].setString( L"Arg_0" );
    pArray[1].setString( L"Arg_1" );
    pArray[2].setString( L"Arg_2" );

    if( argc > 2) {
        Script = StringToOUString( String( argv[2] ) ,  CHARSET_DONTKNOW );
    }

    XInvokationRef xInvokation( x , USR_QUERY );
    XDebuggingRef xDebug( x , USR_QUERY );

    CmdDebugger *pDbg = new CmdDebugger( &xDebug , &xEngine , &xInvokation );

    XEngineListenerRef xDebugRef( (XEngineListener *) pDbg , USR_QUERY);
    xEngine->addEngineListener( xDebugRef );


    if( argc >1 && ! strcmp( argv[1] , "1" ) ) {
        fprintf( stderr, "one thread only\n" );
        Script = UString(   L"print 'Hello World'\n" );
        xEngine->runAsync( Script ,  XInterfaceRef(), args , XEngineListenerRef() );
    }
    else if( argc >1 && ! strcmp( argv[1] , "2" ) )  {

        xEngine->runAsync( UString( L"x=1\nprint 1\n") ,  XInterfaceRef(), args , XEngineListenerRef() );
        xEngine->runAsync( UString( L"x=x+1\nprint 2\n") ,  XInterfaceRef(), args , XEngineListenerRef() );
        xEngine->runAsync( UString( L"x=x+1\nprint 3\n") ,  XInterfaceRef(), args , XEngineListenerRef());
        xEngine->runAsync( UString( L"x=x+1\nprint 4\n") ,  XInterfaceRef(), args , XEngineListenerRef() );



    }
    else if( argc >1 && ! strcmp( argv[1] , "3" ) ) {

        fprintf( stderr , "1st thread in engine y, next 5 threads in engine x\n" );
        yEngine->runAsync( UString( L"print 1\n") ,  XInterfaceRef(), args , XEngineListenerRef() );
        xEngine->runAsync( UString( L"print 2\n") ,  XInterfaceRef(), args , XEngineListenerRef() );
        xEngine->runAsync( UString( L"print 3\n") ,  XInterfaceRef(), args , XEngineListenerRef() );
        xEngine->runAsync( UString( L"print 4\n") ,  XInterfaceRef(), args , XEngineListenerRef());
        xEngine->runAsync( UString( L"print 5\n") ,  XInterfaceRef(), args , XEngineListenerRef());
        xEngine->runAsync( UString( L"print 6\n") ,  XInterfaceRef(), args , XEngineListenerRef());


    }
    pDbg->cmdLine();

    xEngine->removeEngineListener( xDebugRef );

    xReg->revokeImplementation( aDllName, XSimpleRegistryRef() );

    fprintf( stderr, "main terminates\n" );
    return 0;
}

