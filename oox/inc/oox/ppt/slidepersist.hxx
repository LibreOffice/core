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

#ifndef OOX_POWERPOINT_SLIDEPERSIST_HXX
#define OOX_POWERPOINT_SLIDEPERSIST_HXX

#include "tokens.hxx"
#include <boost/shared_ptr.hpp>
#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/drawingml/clrscheme.hxx"
#include "oox/drawingml/textliststyle.hxx"
#include "oox/drawingml/textparagraphproperties.hxx"
#include <oox/ppt/headerfooter.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include "oox/core/fragmenthandler.hxx"

#include <list>

namespace oox { namespace vml { class Drawing; } }

namespace oox { namespace ppt {

enum ShapeLocation
{
    Master,
    Layout,
    Slide
};

// ---------------------------------------------------------------------
class TimeNode;
class SlidePersist;

typedef boost::shared_ptr< SlidePersist > SlidePersistPtr;

class SlidePersist : public boost::enable_shared_from_this< SlidePersist >
{

public:
    SlidePersist( oox::core::XmlFilterBase& rFilter, sal_Bool bMaster, sal_Bool bNotes,
                    const com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage >&,
                    oox::drawingml::ShapePtr pShapesPtr, const ::oox::drawingml::TextListStylePtr & );
    ~SlidePersist();

    com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage >    getPage() const { return mxPage; };

    void setMasterPersist( SlidePersistPtr pMasterPersistPtr ){ mpMasterPagePtr = pMasterPersistPtr; }
    SlidePersistPtr getMasterPersist() const { return mpMasterPagePtr; }

    void setPath( const rtl::OUString& rPath ) { maPath = rPath; }
    const rtl::OUString getPath() const { return maPath; }

    void setLayoutPath( const rtl::OUString& rLayoutPath ) { maLayoutPath = rLayoutPath; }
    const rtl::OUString getLayoutPath() const { return maLayoutPath; }

    void setTheme( const oox::drawingml::ThemePtr pThemePtr ){ mpThemePtr = pThemePtr; }
    oox::drawingml::ThemePtr getTheme() const { return mpThemePtr; }

    void setClrScheme( const oox::drawingml::ClrSchemePtr pClrSchemePtr ){ mpClrSchemePtr = pClrSchemePtr; }
    oox::drawingml::ClrSchemePtr getClrScheme() const { return mpClrSchemePtr; }

    void setClrMap( const oox::drawingml::ClrMapPtr pClrMapPtr ){ mpClrMapPtr = pClrMapPtr; }
    oox::drawingml::ClrMapPtr getClrMap() const { return mpClrMapPtr; }

    void setBackgroundProperties( const oox::drawingml::FillPropertiesPtr pFillPropertiesPtr ){ mpBackgroundPropertiesPtr = pFillPropertiesPtr; }
    oox::drawingml::FillPropertiesPtr getBackgroundProperties() const { return mpBackgroundPropertiesPtr; }

    sal_Bool isMasterPage() const { return mbMaster; }
    sal_Bool isNotesPage() const { return mbNotes; }

    void setLayoutValueToken( sal_Int32 nLayoutValueToken ) { mnLayoutValueToken = nLayoutValueToken; }
    short getLayoutFromValueToken();


    oox::drawingml::TextListStylePtr getDefaultTextStyle() const { return maDefaultTextStylePtr; }
    oox::drawingml::TextListStylePtr getTitleTextStyle() const { return maTitleTextStylePtr; }
    oox::drawingml::TextListStylePtr getBodyTextStyle() const { return maBodyTextStylePtr; }
    oox::drawingml::TextListStylePtr getNotesTextStyle() const { return maNotesTextStylePtr; }
    oox::drawingml::TextListStylePtr getOtherTextStyle() const { return maOtherTextStylePtr; }

    oox::drawingml::ShapePtr getShapes() { return maShapesPtr; }
    ::std::list< boost::shared_ptr< TimeNode > >& getTimeNodeList() { return maTimeNodeList; }
    oox::ppt::HeaderFooter& getHeaderFooter(){ return maHeaderFooter; };

    oox::vml::Drawing* getDrawing() { return mpDrawingPtr.get(); }

    void createXShapes( const oox::core::XmlFilterBase& rFilterBase );
    void createBackground( const oox::core::XmlFilterBase& rFilterBase );
    void applyTextStyles( const oox::core::XmlFilterBase& rFilterBase );

    std::map< ::rtl::OUString, ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > >& getAnimNodesMap() { return maAnimNodesMap; };
    ::oox::drawingml::ShapePtr getShape( const ::rtl::OUString & id ) { return maShapeMap[ id ]; }
    ::oox::drawingml::ShapeIdMap& getShapeMap() { return maShapeMap; }

private:
    rtl::OUString															maPath;
    rtl::OUString															maLayoutPath;
    ::boost::shared_ptr< oox::vml::Drawing >                                mpDrawingPtr;
    com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage >	mxPage;
    oox::drawingml::ThemePtr												mpThemePtr;			// the theme that is used
    oox::drawingml::ClrSchemePtr											mpClrSchemePtr;		// the local color scheme (if any)
    oox::drawingml::ClrMapPtr												mpClrMapPtr;		// color mapping (if any)
    SlidePersistPtr															mpMasterPagePtr;

    oox::drawingml::ShapePtr												maShapesPtr;
    oox::drawingml::FillPropertiesPtr										mpBackgroundPropertiesPtr;
    ::std::list< boost::shared_ptr< TimeNode > >							maTimeNodeList;

    oox::ppt::HeaderFooter													maHeaderFooter;
    sal_Int32																mnLayoutValueToken;
    sal_Bool                                                                mbMaster;
    sal_Bool																mbNotes;

    oox::drawingml::TextListStylePtr										maDefaultTextStylePtr;
    oox::drawingml::TextListStylePtr										maTitleTextStylePtr;
    oox::drawingml::TextListStylePtr										maBodyTextStylePtr;
    oox::drawingml::TextListStylePtr										maNotesTextStylePtr;
    oox::drawingml::TextListStylePtr										maOtherTextStylePtr;

    std::map< ::rtl::OUString, ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > > maAnimNodesMap;
    std::map< ::rtl::OUString, ::oox::drawingml::ShapePtr > maShapeMap;
};

} }

#endif // OOX_POWERPOINT_SLIDEPERSIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
