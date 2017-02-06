/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <recording/dispatchrecorder.hxx>
#include <com/sun/star/frame/DispatchStatement.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <o3tl/any.hxx>
#include <services.h>
#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>
#include <typelib/typedescription.h>

using namespace ::com::sun::star::uno;

namespace framework{

// used to mark a dispatch as comment (mostly it indicates an error) Changing of this define will impact all using of such comments...
#define REM_AS_COMMENT    "rem "

//  XInterface, XTypeProvider, XServiceInfo

DEFINE_XSERVICEINFO_MULTISERVICE_2(
    DispatchRecorder,
    ::cppu::OWeakObject,
    "com.sun.star.frame.DispatchRecorder",
    OUString("com.sun.star.comp.framework.DispatchRecorder"))

DEFINE_INIT_SERVICE(
    DispatchRecorder,
    {
    }
)


void flatten_struct_members(
    ::std::vector< Any > * vec, void const * data,
    typelib_CompoundTypeDescription * pTD )
{
    if (pTD->pBaseTypeDescription)
    {
        flatten_struct_members( vec, data, pTD->pBaseTypeDescription );
    }
    for ( sal_Int32 nPos = 0; nPos < pTD->nMembers; ++nPos )
    {
        vec->push_back(
            Any( static_cast<char const *>(data) + pTD->pMemberOffsets[ nPos ], pTD->ppTypeRefs[ nPos ] ) );
    }
}

Sequence< Any > make_seq_out_of_struct(
    Any const & val )
{
    Type const & type = val.getValueType();
    TypeClass eTypeClass = type.getTypeClass();
    if (TypeClass_STRUCT != eTypeClass && TypeClass_EXCEPTION != eTypeClass)
    {
        throw RuntimeException(
            type.getTypeName() + "is no struct or exception!" );
    }
    typelib_TypeDescription * pTD = nullptr;
    TYPELIB_DANGER_GET( &pTD, type.getTypeLibType() );
    OSL_ASSERT( pTD );
    if (! pTD)
    {
        throw RuntimeException(
            "cannot get type descr of type " + type.getTypeName() );
    }

    ::std::vector< Any > vec;
    vec.reserve( reinterpret_cast<typelib_CompoundTypeDescription *>(pTD)->nMembers ); // good guess
    flatten_struct_members( &vec, val.getValue(), reinterpret_cast<typelib_CompoundTypeDescription *>(pTD) );
    TYPELIB_DANGER_RELEASE( pTD );
    return Sequence< Any >( &vec[ 0 ], vec.size() );
}

DispatchRecorder::DispatchRecorder( const css::uno::Reference< css::uno::XComponentContext >& xContext )
    : m_nRecordingID(0)
    , m_xConverter(css::script::Converter::create(xContext))
{
}

DispatchRecorder::~DispatchRecorder()
{
}

// generate header
void SAL_CALL DispatchRecorder::startRecording( const css::uno::Reference< css::frame::XFrame >& )
{
    /* SAFE{ */
    /* } */
}

void SAL_CALL DispatchRecorder::recordDispatch( const css::util::URL& aURL,
                                                const css::uno::Sequence< css::beans::PropertyValue >& lArguments )
{
    OUString aTarget;

    css::frame::DispatchStatement aStatement( aURL.Complete, aTarget, lArguments, 0, false );
    m_aStatements.push_back( aStatement );
}

void SAL_CALL  DispatchRecorder::recordDispatchAsComment( const css::util::URL& aURL,
                                                          const css::uno::Sequence< css::beans::PropertyValue >& lArguments )
{
    OUString aTarget;

    // last parameter must be set to true -> it's a comment
    css::frame::DispatchStatement aStatement( aURL.Complete, aTarget, lArguments, 0, true );
    m_aStatements.push_back( aStatement );
}

void SAL_CALL DispatchRecorder::endRecording()
{
    SolarMutexGuard g;
    m_aStatements.clear();
}

OUString SAL_CALL DispatchRecorder::getRecordedMacro()
{
    SolarMutexGuard g;

    if ( m_aStatements.empty() )
        return OUString();

    OUStringBuffer aScriptBuffer;
    aScriptBuffer.ensureCapacity(10000);
    m_nRecordingID = 1;

    aScriptBuffer.append("rem ----------------------------------------------------------------------\n");
    aScriptBuffer.append("rem define variables\n");
    aScriptBuffer.append("dim document   as object\n");
    aScriptBuffer.append("dim dispatcher as object\n");
    aScriptBuffer.append("rem ----------------------------------------------------------------------\n");
    aScriptBuffer.append("rem get access to the document\n");
    aScriptBuffer.append("document   = ThisComponent.CurrentController.Frame\n");
    aScriptBuffer.append("dispatcher = createUnoService(\"com.sun.star.frame.DispatchHelper\")\n\n");

    std::vector< css::frame::DispatchStatement>::iterator p;
    for ( p = m_aStatements.begin(); p != m_aStatements.end(); ++p )
        implts_recordMacro( p->aCommand, p->aArgs, p->bIsComment, aScriptBuffer );
    OUString sScript = aScriptBuffer.makeStringAndClear();
    return sScript;
}

void SAL_CALL DispatchRecorder::AppendToBuffer( const css::uno::Any& aValue, OUStringBuffer& aArgumentBuffer )
{
    // if value == bool
    if (aValue.getValueTypeClass() == css::uno::TypeClass_STRUCT )
    {
        // structs are recorded as arrays, convert to "Sequence of any"
        Sequence< Any > aSeq = make_seq_out_of_struct( aValue );
        aArgumentBuffer.append("Array(");
        for ( sal_Int32 nAny=0; nAny<aSeq.getLength(); nAny++ )
        {
            AppendToBuffer( aSeq[nAny], aArgumentBuffer );
            if ( nAny+1 < aSeq.getLength() )
                // not last argument
                aArgumentBuffer.append(",");
        }

        aArgumentBuffer.append(")");
    }
    else if (aValue.getValueTypeClass() == css::uno::TypeClass_SEQUENCE )
    {
        // convert to "Sequence of any"
        css::uno::Sequence < css::uno::Any > aSeq;
        css::uno::Any aNew;
        try { aNew = m_xConverter->convertTo( aValue, cppu::UnoType<css::uno::Sequence < css::uno::Any >>::get() ); }
        catch (const css::uno::Exception&) {}

        aNew >>= aSeq;
        aArgumentBuffer.append("Array(");
        for ( sal_Int32 nAny=0; nAny<aSeq.getLength(); nAny++ )
        {
            AppendToBuffer( aSeq[nAny], aArgumentBuffer );
            if ( nAny+1 < aSeq.getLength() )
                // not last argument
                aArgumentBuffer.append(",");
        }

        aArgumentBuffer.append(")");
    }
    else if (aValue.getValueTypeClass() == css::uno::TypeClass_STRING )
    {
        // strings need \"
        OUString sVal;
        aValue >>= sVal;

        // encode non printable characters or '"' by using the CHR$ function
        if ( !sVal.isEmpty() )
        {
            const sal_Unicode* pChars = sVal.getStr();
            bool bInString = false;
            for ( sal_Int32 nChar=0; nChar<sVal.getLength(); nChar ++ )
            {
                if ( pChars[nChar] < 32 || pChars[nChar] == '"' )
                {
                    // problematic character detected
                    if ( bInString )
                    {
                        // close current string
                        aArgumentBuffer.append("\"");
                        bInString = false;
                    }

                    if ( nChar>0 )
                        // if this is not the first character, parts of the string have already been added
                        aArgumentBuffer.append("+");

                    // add the character constant
                    aArgumentBuffer.append("CHR$(");
                    aArgumentBuffer.append( (sal_Int32) pChars[nChar] );
                    aArgumentBuffer.append(")");
                }
                else
                {
                    if ( !bInString )
                    {
                        if ( nChar>0 )
                            // if this is not the first character, parts of the string have already been added
                            aArgumentBuffer.append("+");

                        // start a new string
                        aArgumentBuffer.append("\"");
                        bInString = true;
                    }

                    aArgumentBuffer.append( pChars[nChar] );
                }
            }

            // close string
            if ( bInString )
                aArgumentBuffer.append("\"");
        }
        else
            aArgumentBuffer.append("\"\"");
    }
    else if (auto nVal = o3tl::tryAccess<sal_Unicode>(aValue))
    {
        // character variables are recorded as strings, back conversion must be handled in client code
        aArgumentBuffer.append("\"");
        if ( (*nVal == '\"') )
            // encode \" to \"\"
            aArgumentBuffer.append(*nVal);
        aArgumentBuffer.append(*nVal);
        aArgumentBuffer.append("\"");
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
        OUString sVal;
        aNew >>= sVal;

        if (aValue.getValueTypeClass() == css::uno::TypeClass_ENUM )
        {
            OUString aName = aValue.getValueType().getTypeName();
            aArgumentBuffer.append( aName );
            aArgumentBuffer.append(".");
        }

        aArgumentBuffer.append(sVal);
    }
}

void SAL_CALL DispatchRecorder::implts_recordMacro( const OUString& aURL,
                                                    const css::uno::Sequence< css::beans::PropertyValue >& lArguments,
                                                          bool bAsComment, OUStringBuffer& aScriptBuffer )
{
    OUStringBuffer aArgumentBuffer(1000);
    OUString       sArrayName;
    // this value is used to name the arrays of aArgumentBuffer
    sArrayName = "args" + OUString::number(m_nRecordingID);

    aScriptBuffer.append("rem ----------------------------------------------------------------------\n");

    sal_Int32 nLength = lArguments.getLength();
    sal_Int32 nValidArgs = 0;
    for( sal_Int32 i=0; i<nLength; ++i )
    {
        if(!lArguments[i].Value.hasValue())
            continue;

        OUStringBuffer sValBuffer(100);
        try
        {
            AppendToBuffer(lArguments[i].Value, sValBuffer);
        }
        catch(const css::uno::Exception&)
        {
            sValBuffer.setLength(0);
        }
        if (sValBuffer.isEmpty())
            continue;

        {
            // add arg().Name
            if(bAsComment)
                aArgumentBuffer.append(REM_AS_COMMENT);
            aArgumentBuffer.append     (sArrayName);
            aArgumentBuffer.append("(");
            aArgumentBuffer.append     (nValidArgs);
            aArgumentBuffer.append(").Name = \"");
            aArgumentBuffer.append     (lArguments[i].Name);
            aArgumentBuffer.append("\"\n");

            // add arg().Value
            if(bAsComment)
                aArgumentBuffer.append(REM_AS_COMMENT);
            aArgumentBuffer.append     (sArrayName);
            aArgumentBuffer.append("(");
            aArgumentBuffer.append     (nValidArgs);
            aArgumentBuffer.append(").Value = ");
            aArgumentBuffer.append     (sValBuffer.makeStringAndClear());
            aArgumentBuffer.append("\n");

            ++nValidArgs;
        }
    }

    // if aArgumentBuffer exist - pack it into the aScriptBuffer
    if(nValidArgs>0)
    {
        if(bAsComment)
            aScriptBuffer.append(REM_AS_COMMENT);
        aScriptBuffer.append("dim ");
        aScriptBuffer.append     (sArrayName);
        aScriptBuffer.append("(");
        aScriptBuffer.append     ((sal_Int32)(nValidArgs-1)); // 0 based!
        aScriptBuffer.append(") as new com.sun.star.beans.PropertyValue\n");
        aScriptBuffer.append     (aArgumentBuffer.makeStringAndClear());
        aScriptBuffer.append("\n");
    }

    // add code for dispatches
    if(bAsComment)
        aScriptBuffer.append(REM_AS_COMMENT);
    aScriptBuffer.append("dispatcher.executeDispatch(document, \"");
    aScriptBuffer.append     (aURL);
    aScriptBuffer.append("\", \"\", 0, ");
    if(nValidArgs<1)
        aScriptBuffer.append("Array()");
    else
    {
        aScriptBuffer.append( sArrayName.getStr() );
        aScriptBuffer.append("()");
    }
    aScriptBuffer.append(")\n\n");

    /* SAFE { */
    m_nRecordingID++;
    /* } */
}

css::uno::Type SAL_CALL DispatchRecorder::getElementType()
{
    return cppu::UnoType<css::frame::DispatchStatement>::get();
}

sal_Bool SAL_CALL DispatchRecorder::hasElements()
{
    return (! m_aStatements.empty());
}

sal_Int32 SAL_CALL DispatchRecorder::getCount()
{
    return m_aStatements.size();
}

css::uno::Any SAL_CALL DispatchRecorder::getByIndex(sal_Int32 idx)
{
    if (idx >= (sal_Int32)m_aStatements.size()) {
        throw css::lang::IndexOutOfBoundsException( "Dispatch recorder out of bounds"  );
   }

    Any element(&m_aStatements[idx],
        cppu::UnoType<css::frame::DispatchStatement>::get());

    return element;
}

void SAL_CALL DispatchRecorder::replaceByIndex(sal_Int32 idx, const css::uno::Any& element)
{
    if (element.getValueType() !=
        cppu::UnoType<css::frame::DispatchStatement>::get()) {
                        throw css::lang::IllegalArgumentException(
                          "Illegal argument in dispatch recorder",
                          Reference< XInterface >(), 2 );
    }

    if (idx >= (sal_Int32)m_aStatements.size()) {
                throw css::lang::IndexOutOfBoundsException(
                        "Dispatch recorder out of bounds"  );

        }

    auto pStatement = o3tl::doAccess<css::frame::DispatchStatement>(element);

    css::frame::DispatchStatement aStatement(
        pStatement->aCommand,
        pStatement->aTarget,
        pStatement->aArgs,
        pStatement->nFlags,
        pStatement->bIsComment);

    m_aStatements[idx] = aStatement;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
