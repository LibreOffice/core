/*************************************************************************
 *
 *  $RCSfile: xmltxtexp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2001-07-20 12:36:12 $
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

/** this file implements an export of a selected EditEngine content into
    a xml stream. See svx/source/inc/xmledit.hxx for interface */

#ifndef _COM_SUN_STAR_UCB_XANYCOMPAREFACTORY_HPP_
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif


#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmloff/xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "xmloff/nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLMETAE_HXX
#include "xmloff/xmlmetae.hxx"
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif

#ifndef _MyEDITENG_HXX
#include "editeng.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmloff/xmlexp.hxx"
#endif

#ifndef _SVX_UNOEDSRC_HXX
#include "unoedsrc.hxx"
#endif

#ifndef _SVX_UNOFORED_HXX
#include "unofored.hxx"
#endif

#ifndef _SVX_UNOTEXT_HXX
#include "unotext.hxx"
#endif

#ifndef _SVX_EDITSOURCE_HXX
#include "editsource.hxx"
#endif

#include <cppuhelper/implbase4.hxx>

#ifndef _SVX_UNOFIELD_HXX
#include "unofield.hxx"
#endif

#ifndef SVX_UNOMOD_HXX
#include "unomod.hxx"
#endif
#include "unonrule.hxx"

using namespace com::sun::star;
using namespace com::sun::star::container;
using namespace com::sun::star::document;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::sax;
using namespace ::rtl;
using namespace cppu;

///////////////////////////////////////////////////////////////////////

class SvxEditEngineSourceImpl;

///////////////////////////////////////////////////////////////////////

class SvxEditEngineSourceImpl
{
private:
    oslInterlockedCount maRefCount;

    EditEngine*             mpEditEngine;
    SvxTextForwarder*       mpTextForwarder;

    ~SvxEditEngineSourceImpl();

public:
    SvxEditEngineSourceImpl( EditEngine* pEditEngine );

    void SAL_CALL acquire();
    void SAL_CALL release();

    SvxTextForwarder*   GetTextForwarder();
};

///////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------------

SvxEditEngineSourceImpl::SvxEditEngineSourceImpl( EditEngine* pEditEngine )
: mpEditEngine( pEditEngine ),
  mpTextForwarder(NULL),
  maRefCount(0)
{
}

//------------------------------------------------------------------------

SvxEditEngineSourceImpl::~SvxEditEngineSourceImpl()
{
    delete mpTextForwarder;
}

//------------------------------------------------------------------------

void SAL_CALL SvxEditEngineSourceImpl::acquire() throw()
{
    osl_incrementInterlockedCount( &maRefCount );
}

//------------------------------------------------------------------------

void SAL_CALL SvxEditEngineSourceImpl::release() throw()
{
    if( ! osl_decrementInterlockedCount( &maRefCount ) )
        delete this;
}

//------------------------------------------------------------------------

SvxTextForwarder* SvxEditEngineSourceImpl::GetTextForwarder()
{
    if (!mpTextForwarder)
        mpTextForwarder = new SvxEditEngineForwarder( *mpEditEngine );

    return mpTextForwarder;
}

// --------------------------------------------------------------------
// SvxTextEditSource
// --------------------------------------------------------------------

SvxEditEngineSource::SvxEditEngineSource( EditEngine* pEditEngine )
{
    mpImpl = new SvxEditEngineSourceImpl( pEditEngine );
    mpImpl->acquire();
}

// --------------------------------------------------------------------

SvxEditEngineSource::SvxEditEngineSource( SvxEditEngineSourceImpl* pImpl )
{
    mpImpl = pImpl;
    mpImpl->acquire();
}

//------------------------------------------------------------------------

SvxEditEngineSource::~SvxEditEngineSource()
{
    mpImpl->release();
}

//------------------------------------------------------------------------

SvxEditSource* SvxEditEngineSource::Clone() const
{
    return new SvxEditEngineSource( mpImpl );
}

//------------------------------------------------------------------------

SvxTextForwarder* SvxEditEngineSource::GetTextForwarder()
{
    if( mpImpl )
        return mpImpl->GetTextForwarder();
    else
        return NULL;
}

//------------------------------------------------------------------------

void SvxEditEngineSource::UpdateData()
{
}


class SvxSimpleUnoModel : public cppu::WeakAggImplHelper4<
                                    ::com::sun::star::frame::XModel,
                                    ::com::sun::star::ucb::XAnyCompareFactory,
                                    ::com::sun::star::style::XStyleFamiliesSupplier,
                                    ::com::sun::star::lang::XMultiServiceFactory >
{
public:
    SvxSimpleUnoModel();
    virtual ~SvxSimpleUnoModel();


    // XMultiServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const ::rtl::OUString& aServiceSpecifier ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XStyleFamiliesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getStyleFamilies(  ) throw(::com::sun::star::uno::RuntimeException);

    // XAnyCompareFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XAnyCompare > SAL_CALL createAnyCompareByName( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

    // XModel
    virtual sal_Bool SAL_CALL attachResource( const ::rtl::OUString& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getURL(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getArgs(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL connectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xController ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disconnectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xController ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL lockControllers(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL unlockControllers(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasControllersLocked(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > SAL_CALL getCurrentController(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCurrentController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xController ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getCurrentSelection(  ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

};

SvxSimpleUnoModel::SvxSimpleUnoModel()
{
}

SvxSimpleUnoModel::~SvxSimpleUnoModel()
{
}

// XMultiServiceFactory ( SvxFmMSFactory )
uno::Reference< uno::XInterface > SAL_CALL SvxSimpleUnoModel::createInstance( const OUString& aServiceSpecifier )
    throw(uno::Exception, uno::RuntimeException)
{
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.text.NumberingRules" ) ) )
    {
        return SvxCreateNumRule( (SdrModel*)NULL );
    }
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.text.TextField.DateTime") ) )
    {
        return (::cppu::OWeakObject * )new SvxUnoTextField( ID_EXT_DATEFIELD );
    }

    return SvxUnoDrawMSFactory::createTextField( aServiceSpecifier );

}

Reference< ::com::sun::star::uno::XInterface > SAL_CALL SvxSimpleUnoModel::createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    return createInstance( ServiceSpecifier );
}

Sequence< ::rtl::OUString > SAL_CALL SvxSimpleUnoModel::getAvailableServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< OUString > aSeq;
    return aSeq;
}

// XAnyCompareFactory
uno::Reference< com::sun::star::ucb::XAnyCompare > SAL_CALL SvxSimpleUnoModel::createAnyCompareByName( const OUString& PropertyName )
    throw(uno::RuntimeException)
{
    return SvxCreateNumRuleCompare();
}

// XStyleFamiliesSupplier
uno::Reference< container::XNameAccess > SAL_CALL SvxSimpleUnoModel::getStyleFamilies(  )
    throw(uno::RuntimeException)
{
    uno::Reference< container::XNameAccess > xStyles;
    return xStyles;
}

// XModel
sal_Bool SAL_CALL SvxSimpleUnoModel::attachResource( const ::rtl::OUString& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw (::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

::rtl::OUString SAL_CALL SvxSimpleUnoModel::getURL(  ) throw (::com::sun::star::uno::RuntimeException)
{
    OUString aStr;
    return aStr;
}

::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL SvxSimpleUnoModel::getArgs(  ) throw (::com::sun::star::uno::RuntimeException)
{
    Sequence< beans::PropertyValue > aSeq;
    return aSeq;
}

void SAL_CALL SvxSimpleUnoModel::connectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xController ) throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SvxSimpleUnoModel::disconnectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xController ) throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SvxSimpleUnoModel::lockControllers(  ) throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SvxSimpleUnoModel::unlockControllers(  ) throw (::com::sun::star::uno::RuntimeException)
{
}

sal_Bool SAL_CALL SvxSimpleUnoModel::hasControllersLocked(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return sal_True;
}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > SAL_CALL SvxSimpleUnoModel::getCurrentController(  ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< frame::XController > xRet;
    return xRet;
}

void SAL_CALL SvxSimpleUnoModel::setCurrentController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xController ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException)
{
}

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SvxSimpleUnoModel::getCurrentSelection(  ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XInterface > xRet;
    return xRet;
}


// XComponent
void SAL_CALL SvxSimpleUnoModel::dispose(  ) throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SvxSimpleUnoModel::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SvxSimpleUnoModel::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException)
{
}

///////////////////////////////////////////////////////////////////////

class SvxXMLTextExportComponent : public SvXMLExport
{
public:
    SvxXMLTextExportComponent(
        EditEngine* pEditEngine,
        const ESelection& rSel,
        const ::rtl::OUString& rFileName,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XDocumentHandler > & xHandler
        );

    ~SvxXMLTextExportComponent();

    // methods without content:
    virtual void _ExportAutoStyles();
    virtual void _ExportMasterStyles();
    virtual void _ExportContent();

private:
    com::sun::star::uno::Reference< com::sun::star::text::XText > mxText;
    EditEngine* mpEditEngine;
    ESelection maSelection;
};

///////////////////////////////////////////////////////////////////////

SvxXMLTextExportComponent::SvxXMLTextExportComponent(
    EditEngine* pEditEngine,
    const ESelection& rSel,
    const ::rtl::OUString& rFileName,
    const com::sun::star::uno::Reference<
        com::sun::star::xml::sax::XDocumentHandler > & xHandler
    )
    : SvXMLExport( rFileName, xHandler, ((frame::XModel*)new SvxSimpleUnoModel()), MAP_CM ),
  mpEditEngine( pEditEngine ),
  maSelection( rSel )
{
    SvxEditEngineSource aEditSource( pEditEngine );

    static const SfxItemPropertyMap SvxXMLTextExportComponentPropertyMap[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
//      SVX_UNOEDIT_OUTLINER_PROPERTIES,
        {MAP_CHAR_LEN(UNO_NAME_NUMBERING_RULES),        EE_PARA_NUMBULLET,  &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace>*)0), 0, 0 },
        {MAP_CHAR_LEN(UNO_NAME_NUMBERING),              EE_PARA_BULLETSTATE,&::getBooleanCppuType(), 0, 0 },
        {MAP_CHAR_LEN(UNO_NAME_NUMBERING_LEVEL),        EE_PARA_OUTLLEVEL,  &::getCppuType((const sal_Int16*)0), 0, 0 },
        SVX_UNOEDIT_PARA_PROPERTIES,
        {0,0}
    };

    SvxUnoText* pUnoText = new SvxUnoText( &aEditSource, SvxXMLTextExportComponentPropertyMap, mxText );
    pUnoText->SetSelection( rSel );
    mxText = pUnoText;

    setExportFlags( EXPORT_AUTOSTYLES|EXPORT_CONTENT );
}

SvxXMLTextExportComponent::~SvxXMLTextExportComponent()
{
}

void SvxWriteXML( EditEngine& rEditEngine, SvStream& rStream, const ESelection& rSel )
{
    try
    {
        do
        {
            // create service factory

            uno::Reference< lang::XMultiServiceFactory> xServiceFactory( ::comphelper::getProcessServiceFactory() );

            if( !xServiceFactory.is() )
            {
                DBG_ERROR( "got no service manager" );
                break;
            }

            // create document handler

            uno::Reference< uno::XInterface > xWriter( xServiceFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer" ) ) ) );

            if( !xWriter.is() )
            {
                DBG_ERROR( "com.sun.star.xml.sax.Writer service missing" );
                break;
            }

            uno::Reference<xml::sax::XDocumentHandler>  xHandler( xWriter, uno::UNO_QUERY );

            // create output stream and active data source
            uno::Reference<io::XOutputStream> xOut( new utl::OOutputStreamWrapper( rStream ) );

/* testcode
            const OUString aURL( RTL_CONSTASCII_USTRINGPARAM( "file:///e:/test.xml" ) );
            SfxMedium aMedium( aURL, STREAM_WRITE | STREAM_TRUNC, TRUE );
            aMedium.IsRemote();
            uno::Reference<io::XOutputStream> xOut( new utl::OOutputStreamWrapper( *aMedium.GetOutStream() ) );
*/


            uno::Reference<io::XActiveDataSource> xMetaSrc( xWriter, uno::UNO_QUERY );
            xMetaSrc->setOutputStream( xOut );

            // export text
            const OUString aName;
            SvxXMLTextExportComponent aExporter( &rEditEngine, rSel, aName, xHandler );

            aExporter.exportDoc();

/* testcode
            aMedium.Commit();
*/

        }
        while( 0 );
    }
    catch( uno::Exception& e )
    {
        DBG_ERROR("exception during xml export");
    }
}

// methods without content:
void SvxXMLTextExportComponent::_ExportAutoStyles()
{
    UniReference< XMLTextParagraphExport > xTextExport( GetTextParagraphExport() );

    xTextExport->collectTextAutoStyles( mxText );
    xTextExport->exportTextAutoStyles();
}

void SvxXMLTextExportComponent::_ExportContent()
{
    UniReference< XMLTextParagraphExport > xTextExport( GetTextParagraphExport() );

    xTextExport->exportText( mxText );
}

void SvxXMLTextExportComponent::_ExportMasterStyles() {}
