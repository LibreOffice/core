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


#include <recording/dispatchrecorder.hxx>
#include <com/sun/star/frame/DispatchStatement.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>
#include <services.h>
#include <vcl/svapp.hxx>
#include <comphelper/componentcontext.hxx>

using namespace ::com::sun::star::uno;

namespace framework{

// used to mark a dispatch as comment (mostly it indicates an error) Changing of this wdefine will impact all using of such comments ...
#define REM_AS_COMMENT    "rem "

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_6(
    DispatchRecorder,
    OWeakObject,
    DIRECT_INTERFACE(css::lang::XTypeProvider),
    DIRECT_INTERFACE(css::lang::XServiceInfo),
    DIRECT_INTERFACE(css::frame::XDispatchRecorder),
    DIRECT_INTERFACE(css::container::XIndexReplace),
    DIRECT_INTERFACE(css::container::XIndexAccess),
    DIRECT_INTERFACE(css::container::XElementAccess))

DEFINE_XTYPEPROVIDER_6(
    DispatchRecorder,
    css::lang::XTypeProvider,
    css::lang::XServiceInfo,
    css::frame::XDispatchRecorder,
    css::container::XIndexReplace,
    css::container::XIndexAccess,
    css::container::XElementAccess)

DEFINE_XSERVICEINFO_MULTISERVICE(
    DispatchRecorder,
    ::cppu::OWeakObject,
    SERVICENAME_DISPATCHRECORDER,
    IMPLEMENTATIONNAME_DISPATCHRECORDER)

DEFINE_INIT_SERVICE(
    DispatchRecorder,
    {
    }
)

#include <typelib/typedescription.h>

//--------------------------------------------------------------------------------------------------
void flatten_struct_members(
    ::std::vector< Any > * vec, void const * data,
    typelib_CompoundTypeDescription * pTD )
    SAL_THROW(())
{
    if (pTD->pBaseTypeDescription)
    {
        flatten_struct_members( vec, data, pTD->pBaseTypeDescription );
    }
    for ( sal_Int32 nPos = 0; nPos < pTD->nMembers; ++nPos )
    {
        vec->push_back(
            Any( (char const *)data + pTD->pMemberOffsets[ nPos ], pTD->ppTypeRefs[ nPos ] ) );
    }
}
//==================================================================================================
Sequence< Any > make_seq_out_of_struct(
    Any const & val )
    SAL_THROW( (RuntimeException) )
{
    Type const & type = val.getValueType();
    TypeClass eTypeClass = type.getTypeClass();
    if (TypeClass_STRUCT != eTypeClass && TypeClass_EXCEPTION != eTypeClass)
    {
        throw RuntimeException(
            type.getTypeName() +
            ::rtl::OUString( "is no struct or exception!" ),
            Reference< XInterface >() );
    }
    typelib_TypeDescription * pTD = 0;
    TYPELIB_DANGER_GET( &pTD, type.getTypeLibType() );
    OSL_ASSERT( pTD );
    if (! pTD)
    {
        throw RuntimeException(
            ::rtl::OUString( "cannot get type descr of type " ) +
            type.getTypeName(),
            Reference< XInterface >() );
    }

    ::std::vector< Any > vec;
    vec.reserve( ((typelib_CompoundTypeDescription *)pTD)->nMembers ); // good guess
    flatten_struct_members( &vec, val.getValue(), (typelib_CompoundTypeDescription *)pTD );
    TYPELIB_DANGER_RELEASE( pTD );
    return Sequence< Any >( &vec[ 0 ], vec.size() );
}

//***********************************************************************
DispatchRecorder::DispatchRecorder( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
        : ThreadHelpBase     ( &Application::GetSolarMutex() )
        , ::cppu::OWeakObject(                               )
        , m_xSMGR            ( xSMGR                         )
        , m_xConverter( css::script::Converter::create(comphelper::ComponentContext(m_xSMGR).getUNOContext()) )
{
}

//************************************************************************
DispatchRecorder::~DispatchRecorder()
{
}

//*************************************************************************
// generate header
void SAL_CALL DispatchRecorder::startRecording( const css::uno::Reference< css::frame::XFrame >& ) throw( css::uno::RuntimeException )
{
    /* SAFE{ */
    /* } */
}

//*************************************************************************
void SAL_CALL DispatchRecorder::recordDispatch( const css::util::URL& aURL,
                                                const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException )
{
    ::rtl::OUString aTarget;

    com::sun::star::frame::DispatchStatement aStatement( aURL.Complete, aTarget, lArguments, 0, sal_False );
    m_aStatements.push_back( aStatement );
}

//*************************************************************************
void SAL_CALL  DispatchRecorder::recordDispatchAsComment( const css::util::URL& aURL,
                                                          const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException )
{
    ::rtl::OUString aTarget;

    // last parameter must be set to true -> it's a comment
        com::sun::star::frame::DispatchStatement aStatement( aURL.Complete, aTarget, lArguments, 0, sal_True );
    m_aStatements.push_back( aStatement );
}

//*************************************************************************
void SAL_CALL DispatchRecorder::endRecording() throw( css::uno::RuntimeException )
{
    /* SAFE{ */
    WriteGuard aWriteLock(m_aLock);
    m_aStatements.clear();
    /* } */
}

//*************************************************************************
::rtl::OUString SAL_CALL DispatchRecorder::getRecordedMacro() throw( css::uno::RuntimeException )
{
    /* SAFE{ */
    WriteGuard aWriteLock(m_aLock);

    if ( m_aStatements.empty() )
        return ::rtl::OUString();

    ::rtl::OUStringBuffer aScriptBuffer;
    aScriptBuffer.ensureCapacity(10000);
    m_nRecordingID = 1;

    aScriptBuffer.appendAscii("rem ----------------------------------------------------------------------\n");
    aScriptBuffer.appendAscii("rem define variables\n");
    aScriptBuffer.appendAscii("dim document   as object\n");
    aScriptBuffer.appendAscii("dim dispatcher as object\n");
    aScriptBuffer.appendAscii("rem ----------------------------------------------------------------------\n");
    aScriptBuffer.appendAscii("rem get access to the document\n");
    aScriptBuffer.appendAscii("document   = ThisComponent.CurrentController.Frame\n");
    aScriptBuffer.appendAscii("dispatcher = createUnoService(\"com.sun.star.frame.DispatchHelper\")\n\n");

    std::vector< com::sun::star::frame::DispatchStatement>::iterator p;
    for ( p = m_aStatements.begin(); p != m_aStatements.end(); ++p )
        implts_recordMacro( p->aCommand, p->aArgs, p->bIsComment, aScriptBuffer );
    ::rtl::OUString sScript = aScriptBuffer.makeStringAndClear();
    return sScript;
    /* } */
}

//*************************************************************************
void SAL_CALL DispatchRecorder::AppendToBuffer( css::uno::Any aValue, ::rtl::OUStringBuffer& aArgumentBuffer )
{
    // if value == bool
    if (aValue.getValueTypeClass() == css::uno::TypeClass_STRUCT )
    {
        // structs are recorded as arrays, convert to "Sequence of any"
        Sequence< Any > aSeq = make_seq_out_of_struct( aValue );
        aArgumentBuffer.appendAscii("Array(");
        for ( sal_Int32 nAny=0; nAny<aSeq.getLength(); nAny++ )
        {
            AppendToBuffer( aSeq[nAny], aArgumentBuffer );
            if ( nAny+1 < aSeq.getLength() )
                // not last argument
                aArgumentBuffer.appendAscii(",");
        }

        aArgumentBuffer.appendAscii(")");
    }
    else if (aValue.getValueTypeClass() == css::uno::TypeClass_SEQUENCE )
    {
        // convert to "Sequence of any"
        css::uno::Sequence < css::uno::Any > aSeq;
        css::uno::Any aNew;
        try { aNew = m_xConverter->convertTo( aValue, ::getCppuType((const css::uno::Sequence < css::uno::Any >*)0) ); }
        catch (const css::uno::Exception&) {}

        aNew >>= aSeq;
        aArgumentBuffer.appendAscii("Array(");
        for ( sal_Int32 nAny=0; nAny<aSeq.getLength(); nAny++ )
        {
            AppendToBuffer( aSeq[nAny], aArgumentBuffer );
            if ( nAny+1 < aSeq.getLength() )
                // not last argument
                aArgumentBuffer.appendAscii(",");
        }

        aArgumentBuffer.appendAscii(")");
    }
    else if (aValue.getValueTypeClass() == css::uno::TypeClass_STRING )
    {
        // strings need \"
        ::rtl::OUString sVal;
        aValue >>= sVal;

        // encode non printable characters or '"' by using the CHR$ function
        if ( !sVal.isEmpty() )
        {
            const sal_Unicode* pChars = sVal.getStr();
            sal_Bool bInString = sal_False;
            for ( sal_Int32 nChar=0; nChar<sVal.getLength(); nChar ++ )
            {
                if ( pChars[nChar] < 32 || pChars[nChar] == '"' )
                {
                    // problematic character detected
                    if ( bInString )
                    {
                        // close current string
                        aArgumentBuffer.appendAscii("\"");
                        bInString = sal_False;
                    }

                    if ( nChar>0 )
                        // if this is not the first character, parts of the string have already been added
                        aArgumentBuffer.appendAscii("+");

                    // add the character constant
                    aArgumentBuffer.appendAscii("CHR$(");
                    aArgumentBuffer.append( (sal_Int32) pChars[nChar] );
                    aArgumentBuffer.appendAscii(")");
                }
                else
                {
                    if ( !bInString )
                    {
                        if ( nChar>0 )
                            // if this is not the first character, parts of the string have already been added
                            aArgumentBuffer.appendAscii("+");

                        // start a new string
                        aArgumentBuffer.appendAscii("\"");
                        bInString = sal_True;
                    }

                    aArgumentBuffer.append( pChars[nChar] );
                }
            }

            // close string
            if ( bInString )
                aArgumentBuffer.appendAscii("\"");
        }
        else
            aArgumentBuffer.appendAscii("\"\"");
    }
    else if (aValue.getValueType() == getCppuCharType())
    {
        // character variables are recorded as strings, back conversion must be handled in client code
        sal_Unicode nVal = *((sal_Unicode*)aValue.getValue());
        aArgumentBuffer.appendAscii("\"");
        if ( (sal_Unicode(nVal) == '\"') )
            // encode \" to \"\"
            aArgumentBuffer.append((sal_Unicode)nVal);
        aArgumentBuffer.append((sal_Unicode)nVal);
        aArgumentBuffer.appendAscii("\"");
    }
    else
    {
        css::uno::Any aNew;
        try
        {
            aNew = m_xConverter->convertToSimpleType( aValue, css::uno::TypeClass_STRING );
        }
        catch (const css::script::CannotConvertException&) {}
        catch (const css::uno::Exception&) {}
        ::rtl::OUString sVal;
        aNew >>= sVal;

        if (aValue.getValueTypeClass() == css::uno::TypeClass_ENUM )
        {
            ::rtl::OUString aName = aValue.getValueType().getTypeName();
            aArgumentBuffer.append( aName );
            aArgumentBuffer.appendAscii(".");
        }

        aArgumentBuffer.append(sVal);
    }
}

void SAL_CALL DispatchRecorder::implts_recordMacro( const ::rtl::OUString& aURL,
                                                    const css::uno::Sequence< css::beans::PropertyValue >& lArguments,
                                                          sal_Bool bAsComment, ::rtl::OUStringBuffer& aScriptBuffer )
{
    ::rtl::OUStringBuffer aArgumentBuffer(1000);
    ::rtl::OUString       sArrayName;
    // this value is used to name the arrays of aArgumentBuffer
    sArrayName = ::rtl::OUString("args");
    sArrayName += ::rtl::OUString::valueOf((sal_Int32)m_nRecordingID);

    aScriptBuffer.appendAscii("rem ----------------------------------------------------------------------\n");

    sal_Int32 nLength = lArguments.getLength();
    sal_Int32 nValidArgs = 0;
    for( sal_Int32 i=0; i<nLength; ++i )
    {
        if(!lArguments[i].Value.hasValue())
            continue;

        ::rtl::OUStringBuffer sValBuffer(100);
        try
        {
            AppendToBuffer(lArguments[i].Value, sValBuffer);
        }
        catch(const css::uno::Exception&)
        {
            sValBuffer.setLength(0);
        }
        if (!sValBuffer.getLength())
            continue;

        {
            // add arg().Name
            if(bAsComment)
                aArgumentBuffer.appendAscii(REM_AS_COMMENT);
            aArgumentBuffer.append     (sArrayName);
            aArgumentBuffer.appendAscii("(");
            aArgumentBuffer.append     (nValidArgs);
            aArgumentBuffer.appendAscii(").Name = \"");
            aArgumentBuffer.append     (lArguments[i].Name);
            aArgumentBuffer.appendAscii("\"\n");

            // add arg().Value
            if(bAsComment)
                aArgumentBuffer.appendAscii(REM_AS_COMMENT);
            aArgumentBuffer.append     (sArrayName);
            aArgumentBuffer.appendAscii("(");
            aArgumentBuffer.append     (nValidArgs);
            aArgumentBuffer.appendAscii(").Value = ");
            aArgumentBuffer.append     (sValBuffer.makeStringAndClear());
            aArgumentBuffer.appendAscii("\n");

            ++nValidArgs;
        }
    }

    // if aArgumentBuffer exist - pack it into the aScriptBuffer
    if(nValidArgs>0)
    {
        if(bAsComment)
            aScriptBuffer.appendAscii(REM_AS_COMMENT);
        aScriptBuffer.appendAscii("dim ");
        aScriptBuffer.append     (sArrayName);
        aScriptBuffer.appendAscii("(");
        aScriptBuffer.append     ((sal_Int32)(nValidArgs-1)); // 0 based!
        aScriptBuffer.appendAscii(") as new com.sun.star.beans.PropertyValue\n");
        aScriptBuffer.append     (aArgumentBuffer.makeStringAndClear());
        aScriptBuffer.appendAscii("\n");
    }

    // add code for dispatches
    if(bAsComment)
        aScriptBuffer.appendAscii(REM_AS_COMMENT);
    aScriptBuffer.appendAscii("dispatcher.executeDispatch(document, \"");
    aScriptBuffer.append     (aURL);
    aScriptBuffer.appendAscii("\", \"\", 0, ");
    if(nValidArgs<1)
        aScriptBuffer.appendAscii("Array()");
    else
    {
        aScriptBuffer.append( sArrayName.getStr() );
        aScriptBuffer.appendAscii("()");
    }
    aScriptBuffer.appendAscii(")\n\n");

    /* SAFE { */
    m_nRecordingID++;
    /* } */
}

com::sun::star::uno::Type SAL_CALL DispatchRecorder::getElementType() throw (::com::sun::star::uno::RuntimeException)
{
    return ::getCppuType((const com::sun::star::frame::DispatchStatement *)NULL);
}

sal_Bool SAL_CALL DispatchRecorder::hasElements()  throw (::com::sun::star::uno::RuntimeException)
{
    return (! m_aStatements.empty());
}

sal_Int32 SAL_CALL DispatchRecorder::getCount() throw (::com::sun::star::uno::RuntimeException)
{
    return m_aStatements.size();
}

com::sun::star::uno::Any SAL_CALL DispatchRecorder::getByIndex(sal_Int32 idx)  throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if (idx >= (sal_Int32)m_aStatements.size()) {
        throw com::sun::star::lang::IndexOutOfBoundsException(
            ::rtl::OUString( "Dispatch recorder out of bounds" ),
                    Reference< XInterface >() );

    }

    Any element(&m_aStatements[idx],
        ::getCppuType((const com::sun::star::frame::DispatchStatement *)NULL));

    return element;
}

void SAL_CALL DispatchRecorder::replaceByIndex(sal_Int32 idx, const com::sun::star::uno::Any& element) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if (element.getValueType() !=
        ::getCppuType((const com::sun::star::frame::DispatchStatement *)NULL)) {
                        throw com::sun::star::lang::IllegalArgumentException(
                        ::rtl::OUString( "Illegal argument in dispatch recorder" ),
                        Reference< XInterface >(), 2 );
    }

    if (idx >= (sal_Int32)m_aStatements.size()) {
                throw com::sun::star::lang::IndexOutOfBoundsException(
                        ::rtl::OUString( "Dispatch recorder out of bounds" ),
                        Reference< XInterface >() );

        }

    com::sun::star::frame::DispatchStatement *pStatement;

    pStatement = (com::sun::star::frame::DispatchStatement *)element.getValue();

    com::sun::star::frame::DispatchStatement aStatement(
        pStatement->aCommand,
        pStatement->aTarget,
        pStatement->aArgs,
        pStatement->nFlags,
        pStatement->bIsComment);

    m_aStatements[idx] = aStatement;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
