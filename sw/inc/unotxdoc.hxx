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
#ifndef _UNOTXDOC_HXX
#define _UNOTXDOC_HXX

#include "swdllapi.h"
#include <sfx2/sfxbasemodel.hxx>

#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XAutoStylesSupplier.hpp>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/document/XRedlinesSupplier.hpp>
#include <com/sun/star/text/XNumberingRulesSupplier.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSettingsSupplier.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/text/XLineNumberingProperties.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#include <com/sun/star/text/XPagePrintable.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XReferenceMarksSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/util/XReplaceable.hpp>
#include <com/sun/star/util/XReplaceDescriptor.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/util/XLinkUpdate.hpp>
#include <com/sun/star/view/XRenderable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/xforms/XFormsSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/text/XFlatParagraphIteratorProvider.hpp>
#include <com/sun/star/document/XDocumentLanguages.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <svl/itemprop.hxx>
#include <svx/fmdmod.hxx>
#include <editeng/UnoForbiddenCharsTable.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <cppuhelper/implbase4.hxx> // helper for implementations
#include <RefreshListenerContainer.hxx>

#include <viewopt.hxx>

#define __IFC32 Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6, Ifc7, Ifc8, Ifc9, Ifc10, Ifc11, Ifc12, Ifc13, Ifc14, Ifc15, Ifc16, \
Ifc17, Ifc18, Ifc19, Ifc20, Ifc21, Ifc22, Ifc23, Ifc24, Ifc25, Ifc26, Ifc27, Ifc28, Ifc29, Ifc30, Ifc31, Ifc32

#define __CLASS_IFC32 class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6, class Ifc7, \
class Ifc8, class Ifc9, class Ifc10, class Ifc11, class Ifc12, class Ifc13, class Ifc14, class Ifc15, class Ifc16, \
class Ifc17, class Ifc18, class Ifc19, class Ifc20, class Ifc21, class Ifc22, class Ifc23, class Ifc24,\
class Ifc25, class Ifc26, class Ifc27, class Ifc28, class Ifc29, class Ifc30, class Ifc31 , class Ifc32

#define __PUBLIC_IFC32 public Ifc1, public Ifc2, public Ifc3, public Ifc4, public Ifc5, public Ifc6, public Ifc7, public Ifc8, public Ifc9, public Ifc10, public Ifc11, public Ifc12, \
public Ifc13, public Ifc14, public Ifc15, public Ifc16, public Ifc17, public Ifc18, \
public Ifc19, public Ifc20, public Ifc21, public Ifc22, public Ifc23, public Ifc24, \
public Ifc25, public Ifc26, public Ifc27, public Ifc28, public Ifc29, public Ifc30, \
public Ifc31, public Ifc32
#include <cppuhelper/implbase_ex.hxx>
#include <cppuhelper/implbase_ex_pre.hxx>
#define __IFC_EX_TYPE_INIT32( class_cast ) \
    __IFC_EX_TYPE_INIT( class_cast, 1 ), __IFC_EX_TYPE_INIT( class_cast, 2 ), \
    __IFC_EX_TYPE_INIT( class_cast, 3 ), __IFC_EX_TYPE_INIT( class_cast, 4 ), \
    __IFC_EX_TYPE_INIT( class_cast, 5 ), __IFC_EX_TYPE_INIT( class_cast, 6 ), \
    __IFC_EX_TYPE_INIT( class_cast, 7 ), __IFC_EX_TYPE_INIT( class_cast, 8 ), \
    __IFC_EX_TYPE_INIT( class_cast, 9 ), __IFC_EX_TYPE_INIT( class_cast, 10 ), \
    __IFC_EX_TYPE_INIT( class_cast, 11 ), __IFC_EX_TYPE_INIT( class_cast, 12 ), \
    __IFC_EX_TYPE_INIT( class_cast, 13 ), __IFC_EX_TYPE_INIT( class_cast, 14 ), \
    __IFC_EX_TYPE_INIT( class_cast, 15 ), __IFC_EX_TYPE_INIT( class_cast, 16 ), \
    __IFC_EX_TYPE_INIT( class_cast, 17 ), __IFC_EX_TYPE_INIT( class_cast, 18 ), \
    __IFC_EX_TYPE_INIT( class_cast, 19 ), __IFC_EX_TYPE_INIT( class_cast, 20 ), \
    __IFC_EX_TYPE_INIT( class_cast, 21 ), __IFC_EX_TYPE_INIT( class_cast, 22 ), \
    __IFC_EX_TYPE_INIT( class_cast, 23 ), __IFC_EX_TYPE_INIT( class_cast, 24 ), \
    __IFC_EX_TYPE_INIT( class_cast, 25 ), __IFC_EX_TYPE_INIT( class_cast, 26 ), \
    __IFC_EX_TYPE_INIT( class_cast, 27 ), __IFC_EX_TYPE_INIT( class_cast, 28 ), \
    __IFC_EX_TYPE_INIT( class_cast, 29 ), __IFC_EX_TYPE_INIT( class_cast, 30 ), \
    __IFC_EX_TYPE_INIT( class_cast, 31 ), __IFC_EX_TYPE_INIT( class_cast, 32)
#include <cppuhelper/implbase_ex_post.hxx>

__DEF_IMPLHELPER_EX( 32 )

namespace css = ::com::sun::star;

class SwDoc;
class SwDocShell;
class UnoActionContext;
class SwXBodyText;
class SwXDrawPage;
class SwUnoCrsr;
class SwXDocumentPropertyHelper;
class SfxViewFrame;
class SwPrintUIOptions;
class SwPrintData;
class SwRenderData;
class ViewShell;

typedef std::deque<UnoActionContext*> ActionContextArr;

typedef cppu::WeakImplHelper32
<
    css::text::XTextDocument,
    css::text::XLineNumberingProperties,
    css::text::XChapterNumberingSupplier,
    css::text::XNumberingRulesSupplier,
    css::text::XFootnotesSupplier,
    css::text::XEndnotesSupplier,
    css::util::XReplaceable,
    css::text::XPagePrintable,
    css::text::XReferenceMarksSupplier,
    css::text::XTextTablesSupplier,
    css::text::XTextFramesSupplier,
    css::text::XBookmarksSupplier,
    css::text::XTextSectionsSupplier,
    css::text::XTextGraphicObjectsSupplier,
    css::text::XTextEmbeddedObjectsSupplier,
    css::text::XTextFieldsSupplier,
    css::style::XStyleFamiliesSupplier,
    css::style::XAutoStylesSupplier,
    css::lang::XServiceInfo,
    css::drawing::XDrawPageSupplier,
    css::text::XDocumentIndexesSupplier,
    css::beans::XPropertySet,
    css::beans::XPropertyState,
    css::document::XLinkTargetSupplier,
    css::document::XRedlinesSupplier,
    css::util::XRefreshable,
    css::util::XLinkUpdate,
    css::view::XRenderable,
    css::xforms::XFormsSupplier,
    css::text::XFlatParagraphIteratorProvider,
    css::document::XDocumentLanguages,
    css::util::XCloneable
>
SwXTextDocumentBaseClass;

class SW_DLLPUBLIC SwXTextDocument : public SwXTextDocumentBaseClass,
    public SvxFmMSFactory,
    public SfxBaseModel
{
    ActionContextArr        aActionArr;
    SwRefreshListenerContainer  aRefreshCont;

    const SfxItemPropertySet* pPropSet;

    SwDocShell*             pDocShell;
    sal_Bool                    bObjectValid;

    SwXDrawPage*            pDrawPage;
    css::uno::Reference< css::drawing::XDrawPage > *            pxXDrawPage;

    css::uno::Reference< css::text::XText >                 xBodyText;
    SwXBodyText*            pBodyText;
    css::uno::Reference< css::uno::XAggregation >           xNumFmtAgg;

    css::uno::Reference< css::container::XIndexAccess > *     pxXNumberingRules;
    css::uno::Reference< css::container::XIndexAccess > *     pxXFootnotes;
    css::uno::Reference< css::beans::XPropertySet > *        pxXFootnoteSettings;
    css::uno::Reference< css::container::XIndexAccess > *       pxXEndnotes;
    css::uno::Reference< css::beans::XPropertySet > *        pxXEndnoteSettings;
    css::uno::Reference< css::container::XNameAccess > *            pxXReferenceMarks;
    css::uno::Reference< css::container::XEnumerationAccess > * pxXTextFieldTypes;
    css::uno::Reference< css::container::XNameAccess > *            pxXTextFieldMasters;
    css::uno::Reference< css::container::XNameAccess > *            pxXTextSections;
    css::uno::Reference< css::container::XNameAccess > *            pxXBookmarks;
    css::uno::Reference< css::container::XNameAccess > *            pxXTextTables;
    css::uno::Reference< css::container::XNameAccess > *            pxXTextFrames;
    css::uno::Reference< css::container::XNameAccess > *            pxXGraphicObjects;
    css::uno::Reference< css::container::XNameAccess > *            pxXEmbeddedObjects;
    css::uno::Reference< css::container::XNameAccess > *            pxXStyleFamilies;
    mutable css::uno::Reference< css::style::XAutoStyles > *  pxXAutoStyles;
    css::uno::Reference< css::container::XIndexReplace > *        pxXChapterNumbering;
    css::uno::Reference< css::container::XIndexAccess > *       pxXDocumentIndexes;

    css::uno::Reference< css::beans::XPropertySet > *       pxXLineNumberingProperties;
    css::uno::Reference< css::container::XNameAccess > *            pxLinkTargetSupplier;
    css::uno::Reference< css::container::XEnumerationAccess >*  pxXRedlines;
    css::uno::Reference< css::container::XNameContainer>        xXFormsContainer;

    //temporary frame to enable PDF export if no valid view is available
    SfxViewFrame*                                   m_pHiddenViewFrame;
    css::uno::Reference< css::uno::XInterface>      xPropertyHelper;
    SwXDocumentPropertyHelper*                      pPropertyHelper;

    SwPrintUIOptions *                              m_pPrintUIOptions;
    SwRenderData *                               m_pRenderData;

    void                    GetBodyText();
    void                    GetNumberFormatter();

    // used for XRenderable implementation
    SfxViewShell *  GuessViewShell( /* out */ bool &rbIsSwSrcView, const css::uno::Reference< css::frame::XController > xController = css::uno::Reference< css::frame::XController >() );
    SwDoc *         GetRenderDoc( SfxViewShell *&rpView, const css::uno::Any& rSelection, bool bIsPDFExport );
    SfxViewShell *  GetRenderView( bool &rbIsSwSrcView, const css::uno::Sequence< css::beans::PropertyValue >& rxOptions, bool bIsPDFExport );

    rtl::OUString           maBuildId;

    using SfxBaseModel::addEventListener;
    using SfxBaseModel::removeEventListener;

protected:
    virtual ~SwXTextDocument();
public:
    SwXTextDocument(SwDocShell* pShell);

    inline void notifyRefreshListeners() { aRefreshCont.Refreshed(); }
    virtual     css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw(css::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw();
    virtual void SAL_CALL release(  ) throw();

    //XWeak
    virtual css::uno::Reference< css::uno::XAdapter > SAL_CALL queryAdapter(  ) throw(css::uno::RuntimeException);

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException);

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException);

    //XTextDocument
    virtual css::uno::Reference< css::text::XText >  SAL_CALL getText(void) throw( css::uno::RuntimeException );
    virtual void SAL_CALL reformat(void) throw( css::uno::RuntimeException );

    //XModel
    virtual sal_Bool SAL_CALL attachResource( const ::rtl::OUString& aURL, const css::uno::Sequence< css::beans::PropertyValue >& aArgs ) throw(css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getURL(  ) throw(css::uno::RuntimeException);
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getArgs(  ) throw(css::uno::RuntimeException);
    virtual void SAL_CALL connectController( const css::uno::Reference< css::frame::XController >& xController ) throw(css::uno::RuntimeException);
    virtual void SAL_CALL disconnectController( const css::uno::Reference< css::frame::XController >& xController ) throw(css::uno::RuntimeException);
    virtual void SAL_CALL lockControllers(  ) throw(css::uno::RuntimeException);
    virtual void SAL_CALL unlockControllers(  ) throw(css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasControllersLocked(  ) throw(css::uno::RuntimeException);
    virtual css::uno::Reference< css::frame::XController > SAL_CALL getCurrentController(  ) throw(css::uno::RuntimeException);
    virtual void SAL_CALL setCurrentController( const css::uno::Reference< css::frame::XController >& xController ) throw(css::container::NoSuchElementException, css::uno::RuntimeException);
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getCurrentSelection(  ) throw(css::uno::RuntimeException);

    //XComponent
    virtual void SAL_CALL dispose(void) throw( css::uno::RuntimeException );
    virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw( css::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw( css::uno::RuntimeException );

    //XCloseable
    virtual void SAL_CALL close( sal_Bool bDeliverOwnership ) throw (css::util::CloseVetoException, css::uno::RuntimeException);

    //XLineNumberingProperties
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getLineNumberingProperties(void) throw( css::uno::RuntimeException );

    //XChapterNumberingSupplier
    virtual css::uno::Reference< css::container::XIndexReplace >  SAL_CALL getChapterNumberingRules(void) throw( css::uno::RuntimeException );

    //XNumberingRulesSupplier
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL getNumberingRules() throw (css::uno::RuntimeException);

    //XFootnotesSupplier
    virtual css::uno::Reference< css::container::XIndexAccess >  SAL_CALL getFootnotes(void) throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet >  SAL_CALL getFootnoteSettings(void) throw( css::uno::RuntimeException );

    //XEndnotesSupplier
    virtual css::uno::Reference< css::container::XIndexAccess >  SAL_CALL getEndnotes(void) throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet >  SAL_CALL getEndnoteSettings(void) throw( css::uno::RuntimeException );

    //XReplaceable
    virtual css::uno::Reference< css::util::XReplaceDescriptor >  SAL_CALL createReplaceDescriptor(void) throw( css::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL replaceAll(const css::uno::Reference< css::util::XSearchDescriptor > & xDesc) throw( css::uno::RuntimeException );

    //XSearchable
    virtual css::uno::Reference< css::util::XSearchDescriptor >  SAL_CALL createSearchDescriptor(void) throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::container::XIndexAccess >  SAL_CALL findAll(const css::uno::Reference< css::util::XSearchDescriptor > & xDesc) throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::uno::XInterface >  SAL_CALL findFirst(const css::uno::Reference< css::util::XSearchDescriptor > & xDesc) throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::uno::XInterface >  SAL_CALL findNext(const css::uno::Reference< css::uno::XInterface > & xStartAt, const css::uno::Reference< css::util::XSearchDescriptor > & xDesc) throw( css::uno::RuntimeException );

    //XPagePrintable
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getPagePrintSettings(void) throw( css::uno::RuntimeException );
    virtual void SAL_CALL setPagePrintSettings(const css::uno::Sequence< css::beans::PropertyValue >& aSettings) throw( css::uno::RuntimeException );
    virtual void SAL_CALL printPages(const css::uno::Sequence< css::beans::PropertyValue >& xOptions) throw( css::lang::IllegalArgumentException, css::uno::RuntimeException );

    //XReferenceMarksSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getReferenceMarks(void) throw( css::uno::RuntimeException );

    // css::text::XTextFieldsSupplier
    virtual css::uno::Reference< css::container::XEnumerationAccess >  SAL_CALL getTextFields(void) throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getTextFieldMasters(void) throw( css::uno::RuntimeException );

    // css::text::XTextEmbeddedObjectsSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getEmbeddedObjects(void) throw( css::uno::RuntimeException );

    // css::text::XBookmarksSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getBookmarks(void) throw( css::uno::RuntimeException );

    // css::text::XTextSectionsSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getTextSections(void) throw( css::uno::RuntimeException );

    // css::text::XTextTablesSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getTextTables(void) throw( css::uno::RuntimeException );

    // css::text::XTextGraphicObjectsSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getGraphicObjects(void) throw( css::uno::RuntimeException );

    // css::text::XTextFramesSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getTextFrames(void) throw( css::uno::RuntimeException );

    //XStyleFamiliesSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getStyleFamilies(void) throw( css::uno::RuntimeException );

    //XAutoStylesSupplier
    virtual css::uno::Reference< css::style::XAutoStyles > SAL_CALL getAutoStyles(  ) throw (css::uno::RuntimeException);

    //XMultiServiceFactory
    virtual css::uno::Reference< css::uno::XInterface >  SAL_CALL createInstance(const rtl::OUString& ServiceSpecifier)
                throw( css::uno::Exception, css::uno::RuntimeException );
    virtual css::uno::Reference< css::uno::XInterface >  SAL_CALL createInstanceWithArguments(const rtl::OUString& ServiceSpecifier,
                const css::uno::Sequence< css::uno::Any >& Arguments)
                throw( css::uno::Exception, css::uno::RuntimeException );
    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getAvailableServiceNames(void)
                throw( css::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( css::uno::RuntimeException );

    // css::drawing::XDrawPageSupplier
    virtual css::uno::Reference< css::drawing::XDrawPage >  SAL_CALL getDrawPage(void) throw( css::uno::RuntimeException );

    // css::text::XDocumentIndexesSupplier
    virtual css::uno::Reference< css::container::XIndexAccess >  SAL_CALL getDocumentIndexes(void) throw( css::uno::RuntimeException );

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);

    //XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& rPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException);
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< ::rtl::OUString >& rPropertyNames ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& rPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& rPropertyName ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);

    //XLinkTargetSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getLinks(void) throw( css::uno::RuntimeException );

    //XRedlinesSupplier
    virtual css::uno::Reference< css::container::XEnumerationAccess > SAL_CALL getRedlines(  ) throw(css::uno::RuntimeException);

    // css::util::XRefreshable
    virtual void SAL_CALL refresh(void) throw( css::uno::RuntimeException );
    virtual void SAL_CALL addRefreshListener(const css::uno::Reference< css::util::XRefreshListener > & l) throw( css::uno::RuntimeException );
    virtual void SAL_CALL removeRefreshListener(const css::uno::Reference< css::util::XRefreshListener > & l) throw( css::uno::RuntimeException );

    // css::util::XLinkUpdate,
    virtual void SAL_CALL updateLinks(  ) throw(css::uno::RuntimeException);

    // css::view::XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount( const css::uno::Any& aSelection, const css::uno::Sequence< css::beans::PropertyValue >& xOptions ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getRenderer( sal_Int32 nRenderer, const css::uno::Any& aSelection, const css::uno::Sequence< css::beans::PropertyValue >& xOptions ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);
    virtual void SAL_CALL render( sal_Int32 nRenderer, const css::uno::Any& aSelection, const css::uno::Sequence< css::beans::PropertyValue >& xOptions ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

    // css::xforms::XFormsSupplier
    virtual css::uno::Reference< css::container::XNameContainer > SAL_CALL getXForms(  ) throw (css::uno::RuntimeException);

    // css::document::XDocumentLanguages
    virtual css::uno::Sequence< css::lang::Locale > SAL_CALL getDocumentLanguages( ::sal_Int16 nScriptTypes, ::sal_Int16 nCount ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

    // css::text::XFlatParagraphIteratorProvider:
    virtual css::uno::Reference< css::text::XFlatParagraphIterator > SAL_CALL getFlatParagraphIterator(::sal_Int32 nTextMarkupType, sal_Bool bAutomatic ) throw (css::uno::RuntimeException);

    // ::com::sun::star::util::XCloneable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException);


    //
    void                        Invalidate();
    void                        Reactivate(SwDocShell* pNewDocShell);
    SwXDocumentPropertyHelper * GetPropertyHelper ();
    sal_Bool                    IsValid() const {return bObjectValid;}

    void                        InitNewDoc();

    SwUnoCrsr*                  CreateCursorForSearch(css::uno::Reference< css::text::XTextCursor > & xCrsr);
    SwUnoCrsr*                  FindAny(const css::uno::Reference< css::util::XSearchDescriptor > & xDesc,
                                            css::uno::Reference< css::text::XTextCursor > & xCrsr, sal_Bool bAll,
                                            sal_Int32& nResult,
                                            css::uno::Reference< css::uno::XInterface >  xLastResult);

    SwXDrawPage*                GetDrawPage();
    SwDocShell*                 GetDocShell() {return pDocShell;}


    void * SAL_CALL operator new( size_t ) throw();
    void SAL_CALL operator delete( void * ) throw();

};

class SwXLinkTargetSupplier : public cppu::WeakImplHelper2
<
    css::container::XNameAccess,
    css::lang::XServiceInfo
>
{
    SwXTextDocument* pxDoc;
    String sTables;
    String sFrames;
    String sGraphics;
    String sOLEs;
    String sSections;
    String sOutlines;
    String sBookmarks;

public:
    SwXLinkTargetSupplier(SwXTextDocument& rxDoc);
    ~SwXLinkTargetSupplier();

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const rtl::OUString& Name)  throw( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException );
    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getElementNames(void) throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const rtl::OUString& Name) throw( css::uno::RuntimeException );

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( css::uno::RuntimeException );

    //
    void    Invalidate() {pxDoc = 0;}
};

class SwXLinkNameAccessWrapper : public cppu::WeakImplHelper4
<
    css::beans::XPropertySet,
    css::container::XNameAccess,
    css::lang::XServiceInfo,
    css::document::XLinkTargetSupplier
>
{
    css::uno::Reference< css::container::XNameAccess >    xRealAccess;
    const SfxItemPropertySet*                                                       pPropSet;
    const String                                                                    sLinkSuffix;
    const String                                                                    sLinkDisplayName;
    css::uno::Reference< css::text::XTextDocument >         xDoc;
    SwXTextDocument*                                                                pxDoc;


public:
    SwXLinkNameAccessWrapper(css::uno::Reference< css::container::XNameAccess >  xAccess,
            const String& rLinkDisplayName, String sSuffix);
    SwXLinkNameAccessWrapper(SwXTextDocument& rxDoc,
            const String& rLinkDisplayName, String sSuffix);
    ~SwXLinkNameAccessWrapper();

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const rtl::OUString& Name)  throw( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException );
    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getElementNames(void) throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const rtl::OUString& Name) throw( css::uno::RuntimeException );

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException);

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);

    //XLinkTargetSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getLinks(void) throw( css::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( css::uno::RuntimeException );

};

class SwXOutlineTarget : public cppu::WeakImplHelper2
<
    css::beans::XPropertySet,
    css::lang::XServiceInfo
>
{
    const SfxItemPropertySet*   pPropSet;
    String                      sOutlineText;

public:
    SwXOutlineTarget(const String& rOutlineText);
    ~SwXOutlineTarget();

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( css::uno::RuntimeException );
};
#endif

class SwXDocumentPropertyHelper : public SvxUnoForbiddenCharsTable
{
    css::uno::Reference < css::uno::XInterface > xDashTable;
    css::uno::Reference < css::uno::XInterface > xGradientTable;
    css::uno::Reference < css::uno::XInterface > xHatchTable;
    css::uno::Reference < css::uno::XInterface > xBitmapTable;
    css::uno::Reference < css::uno::XInterface > xTransGradientTable;
    css::uno::Reference < css::uno::XInterface > xMarkerTable;
    css::uno::Reference < css::uno::XInterface > xDrawDefaults;

    SwDoc*  m_pDoc;
public:
    SwXDocumentPropertyHelper(SwDoc& rDoc);
    ~SwXDocumentPropertyHelper();
    css::uno::Reference<css::uno::XInterface> GetDrawTable(short nWhich);
    void Invalidate();

    virtual void onChange();
};


/*  The class SwViewOptionAdjust_Impl is used to adjust the SwViewOption of
    the current ViewShell so that fields are not printed as commands and
    hidden characters are always invisible. Hidden text and place holders
    should be printed according to the current print options.
    After printing the view options are restored
  -----------------------------------------------------------------------*/
class SwViewOptionAdjust_Impl
{
    ViewShell *     m_pShell;
    SwViewOption    m_aOldViewOptions;

public:
    SwViewOptionAdjust_Impl( ViewShell& rSh, const SwViewOption &rViewOptions );
    ~SwViewOptionAdjust_Impl();
    void AdjustViewOptions( SwPrintData const* const pPrtOptions );
    bool checkShell( const ViewShell& rCompare ) const
    { return &rCompare == m_pShell; }
    void DontTouchThatViewShellItSmellsFunny() { m_pShell = 0; }
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
