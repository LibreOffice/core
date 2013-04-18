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

#ifndef OOX_POWERPOINT_SLIDEPERSIST_HXX
#define OOX_POWERPOINT_SLIDEPERSIST_HXX

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
#include "oox/ppt/comments.hxx"

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

#if OSL_DEBUG_LEVEL > 0
    static com::sun::star::uno::WeakReference< com::sun::star::drawing::XDrawPage > mxDebugPage;
#endif

    void setMasterPersist( SlidePersistPtr pMasterPersistPtr ){ mpMasterPagePtr = pMasterPersistPtr; }
    SlidePersistPtr getMasterPersist() const { return mpMasterPagePtr; }

    void setPath( const OUString& rPath ) { maPath = rPath; }
    const OUString getPath() const { return maPath; }

    void setLayoutPath( const OUString& rLayoutPath ) { maLayoutPath = rLayoutPath; }
    const OUString getLayoutPath() const { return maLayoutPath; }

    void setTheme( const oox::drawingml::ThemePtr pThemePtr ){ mpThemePtr = pThemePtr; }
    oox::drawingml::ThemePtr getTheme() const { return mpThemePtr; }

    void setClrScheme( const oox::drawingml::ClrSchemePtr pClrSchemePtr ){ mpClrSchemePtr = pClrSchemePtr; }
    oox::drawingml::ClrSchemePtr getClrScheme() const { return mpClrSchemePtr; }

    void setClrMap( const oox::drawingml::ClrMapPtr pClrMapPtr ){ mpClrMapPtr = pClrMapPtr; }
    oox::drawingml::ClrMapPtr getClrMap() const { return mpClrMapPtr; }

    void setBackgroundProperties( const oox::drawingml::FillPropertiesPtr pFillPropertiesPtr ){ mpBackgroundPropertiesPtr = pFillPropertiesPtr; }
    oox::drawingml::FillPropertiesPtr getBackgroundProperties() const { return mpBackgroundPropertiesPtr; }
    oox::drawingml::Color& getBackgroundColor() { return maBackgroundColor; }

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
    void hideShapesAsMasterShapes();
    ::std::list< boost::shared_ptr< TimeNode > >& getTimeNodeList() { return maTimeNodeList; }
    oox::ppt::HeaderFooter& getHeaderFooter(){ return maHeaderFooter; };

    oox::vml::Drawing* getDrawing() { return mpDrawingPtr.get(); }

    void createXShapes( oox::core::XmlFilterBase& rFilterBase );
    void createBackground( const oox::core::XmlFilterBase& rFilterBase );
    void applyTextStyles( const oox::core::XmlFilterBase& rFilterBase );

    std::map< OUString, ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > >& getAnimNodesMap() { return maAnimNodesMap; };
    ::oox::drawingml::ShapePtr getShape( const OUString & id ) { return maShapeMap[ id ]; }
    ::oox::drawingml::ShapeIdMap& getShapeMap() { return maShapeMap; }

    CommentList& getCommentsList() { return maCommentsList; }
    CommentAuthorList& getCommentAuthors() { return maCommentAuthors; }

private:
    OUString                                                           maPath;
    OUString                                                           maLayoutPath;
    ::boost::shared_ptr< oox::vml::Drawing >                                mpDrawingPtr;
    com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage >    mxPage;
    oox::drawingml::ThemePtr                                                mpThemePtr;         // the theme that is used
    oox::drawingml::ClrSchemePtr                                            mpClrSchemePtr;     // the local color scheme (if any)
    oox::drawingml::ClrMapPtr                                               mpClrMapPtr;        // color mapping (if any)
    SlidePersistPtr                                                         mpMasterPagePtr;

    oox::drawingml::ShapePtr                                                maShapesPtr;
    oox::drawingml::Color                                                   maBackgroundColor;
    oox::drawingml::FillPropertiesPtr                                       mpBackgroundPropertiesPtr;
    ::std::list< boost::shared_ptr< TimeNode > >                            maTimeNodeList;

    oox::ppt::HeaderFooter                                                  maHeaderFooter;
    sal_Int32                                                               mnLayoutValueToken;
    sal_Bool                                                                mbMaster;
    sal_Bool                                                                mbNotes;

    oox::drawingml::TextListStylePtr                                        maDefaultTextStylePtr;
    oox::drawingml::TextListStylePtr                                        maTitleTextStylePtr;
    oox::drawingml::TextListStylePtr                                        maBodyTextStylePtr;
    oox::drawingml::TextListStylePtr                                        maNotesTextStylePtr;
    oox::drawingml::TextListStylePtr                                        maOtherTextStylePtr;

    std::map< OUString, ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > > maAnimNodesMap;
    std::map< OUString, ::oox::drawingml::ShapePtr > maShapeMap;

    // slide comments
    CommentList                                                             maCommentsList;
    CommentAuthorList                                                       maCommentAuthors;
};

} }

#endif // OOX_POWERPOINT_SLIDEPERSIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
