/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include <SwAppletImpl.hxx>
#include <svtools/htmlkywd.hxx>
#include <svl/urihelper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>

#include <comphelper/embeddedobjectcontainer.hxx>
#include <sot/clsids.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <svtools/embedhlp.hxx>

using namespace com::sun::star;

namespace {

static char const sHTML_O_archive[] = "ARCHIVE";
static char const sHTML_O_Archives[] = "ARCHIVES";
static char const sHTML_O_Object[] = "OBJECT";

}

sal_uInt16 SwApplet_Impl::GetOptionType( const String& rName, sal_Bool bApplet )
{
    sal_uInt16 nType = bApplet ? SWHTML_OPTTYPE_PARAM : SWHTML_OPTTYPE_TAG;

    switch( rName.GetChar(0) )
    {
    case 'A':
    case 'a':
        if( rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_align ) ||
            rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_alt ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        else if( bApplet &&
                 (rName.EqualsIgnoreCaseAscii( sHTML_O_archive ) ||
                 rName.EqualsIgnoreCaseAscii( sHTML_O_Archives )) )
            nType = SWHTML_OPTTYPE_TAG;
        break;
    case 'C':
    case 'c':
        if( rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_class ) ||
            (bApplet && (rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_code ) ||
                         rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_codebase ))) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'H':
    case 'h':
        if( rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_height ) )
            nType = SWHTML_OPTTYPE_SIZE;
        else if( rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_hspace ) ||
            (!bApplet && rName.EqualsIgnoreCaseAscii( OOO_STRING_SW_HTML_O_Hidden )) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'I':
    case 'i':
        if( rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_id ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'M':
    case 'm':
        if( bApplet && rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_mayscript ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'N':
    case 'n':
        if( rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_name ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'O':
    case 'o':
        if( bApplet && rName.EqualsIgnoreCaseAscii( sHTML_O_Object ) )
            nType = SWHTML_OPTTYPE_TAG;
        break;
    case 'S':
    case 's':
        if( rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_style ) ||
            (!bApplet && rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_src )) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'T':
    case 't':
        if( !bApplet && rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_type ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'V':
    case 'v':
        if( rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_vspace ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'W':
    case 'w':
        if( rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_width ) )
            nType = SWHTML_OPTTYPE_SIZE;
        break;
    }

    return nType;
}
SwApplet_Impl::SwApplet_Impl( SfxItemPool& rPool, sal_uInt16 nWhich1, sal_uInt16 nWhich2 ) :
        aItemSet( rPool, nWhich1, nWhich2 )
{
}

void SwApplet_Impl::CreateApplet( const String& rCode, const String& rName,
                                      sal_Bool bMayScript, const String& rCodeBase,
                                      const String& rDocumentBaseURL )
{
    comphelper::EmbeddedObjectContainer aCnt;
    ::rtl::OUString aName;

    // create Applet; it will be in running state
    xApplet = aCnt.CreateEmbeddedObject( SvGlobalName( SO3_APPLET_CLASSID ).GetByteSequence(), aName );
    ::svt::EmbeddedObjectRef::TryRunningState( xApplet );

    INetURLObject aUrlBase(rDocumentBaseURL);
    aUrlBase.removeSegment();

    String sDocBase = aUrlBase.GetMainURL(INetURLObject::NO_DECODE);
    uno::Reference < beans::XPropertySet > xSet( xApplet->getComponent(), uno::UNO_QUERY );
    if ( xSet.is() )
    {
        xSet->setPropertyValue( ::rtl::OUString::createFromAscii("AppletCode"), uno::makeAny( ::rtl::OUString( rCode ) ) );
        xSet->setPropertyValue( ::rtl::OUString::createFromAscii("AppletName"), uno::makeAny( ::rtl::OUString( rName ) ) );
        xSet->setPropertyValue( ::rtl::OUString::createFromAscii("AppletIsScript"), uno::makeAny( sal_Bool(bMayScript) ) );
        xSet->setPropertyValue( ::rtl::OUString::createFromAscii("AppletDocBase"), uno::makeAny( ::rtl::OUString(sDocBase) ) );
        if ( rCodeBase.Len() )
            xSet->setPropertyValue( ::rtl::OUString::createFromAscii("AppletCodeBase"), uno::makeAny( ::rtl::OUString( rCodeBase ) ) );
        else
            xSet->setPropertyValue( ::rtl::OUString::createFromAscii("AppletCodeBase"), uno::makeAny( ::rtl::OUString( sDocBase ) ) );
    }
}
#ifdef SOLAR_JAVA
sal_Bool SwApplet_Impl::CreateApplet( const String& rBaseURL )
{
    String aCode, aName, aCodeBase;
    sal_Bool bMayScript = sal_False;

    sal_uInt32 nArgCount = aCommandList.Count();
    for( sal_uInt32 i=0; i<nArgCount; i++ )
    {
        const SvCommand& rArg = aCommandList[i];
        const String& rName = rArg.GetCommand();
        if( rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_code ) )
            aCode = rArg.GetArgument();
        else if( rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_codebase ) )
            aCodeBase = INetURLObject::GetAbsURL( rBaseURL, rArg.GetArgument() );
        else if( rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_name ) )
            aName = rArg.GetArgument();
        else if( rName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_O_mayscript ) )
            bMayScript = sal_True;
    }

    if( !aCode.Len() )
        return sal_False;
    CreateApplet( aCode, aName, bMayScript, aCodeBase, rBaseURL );
    return sal_True;
}
#endif

SwApplet_Impl::~SwApplet_Impl()
{
}
void SwApplet_Impl::FinishApplet()
{
    //xApplet->EnableSetModified( sal_True );
    uno::Reference < beans::XPropertySet > xSet( xApplet->getComponent(), uno::UNO_QUERY );
    if ( xSet.is() )
    {
        uno::Sequence < beans::PropertyValue > aProps;
        aCommandList.FillSequence( aProps );
        xSet->setPropertyValue( ::rtl::OUString::createFromAscii("AppletCommands"), uno::makeAny( aProps ) );
    }
}

#ifdef SOLAR_JAVA
void SwApplet_Impl::AppendParam( const String& rName, const String& rValue )
{
    aCommandList.Append( rName, rValue );
}
#endif
