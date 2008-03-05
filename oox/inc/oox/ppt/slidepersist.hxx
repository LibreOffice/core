/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slidepersist.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:57:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef OOX_POWERPOINT_SLIDEPERSIST_HXX
#define OOX_POWERPOINT_SLIDEPERSIST_HXX

#include "tokens.hxx"
#include <boost/shared_ptr.hpp>
#include <oox/vml/drawing.hxx>
#include <oox/drawingml/shape.hxx>
#include <oox/drawingml/theme.hxx>
#include <oox/drawingml/clrscheme.hxx>
#include <oox/drawingml/textliststyle.hxx>
#include <oox/drawingml/textparagraphproperties.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include "oox/core/fragmenthandler.hxx"

#include <list>

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
    SlidePersist( sal_Bool bMaster, sal_Bool bNotes, const com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage >&,
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

    oox::drawingml::TextListStylePtr getTitleTextStyle() const { return maTitleTextStylePtr; }
    oox::drawingml::TextListStylePtr getBodyTextStyle() const { return maBodyTextStylePtr; }
    oox::drawingml::TextListStylePtr getNotesTextStyle() const { return maNotesTextStylePtr; }
    oox::drawingml::TextListStylePtr getOtherTextStyle() const { return maOtherTextStylePtr; }

    oox::drawingml::ShapePtr getShapes() { return maShapesPtr; }
    ::std::list< boost::shared_ptr< TimeNode > >& getTimeNodeList() { return maTimeNodeList; }

    oox::vml::DrawingPtr getDrawing() { return mpDrawingPtr; }

    void createXShapes( const oox::core::XmlFilterBase& rFilterBase );
    void createBackground( const oox::core::XmlFilterBase& rFilterBase );
    void applyTextStyles( const oox::core::XmlFilterBase& rFilterBase );

    std::map< ::rtl::OUString, ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > >& getAnimNodesMap() { return maAnimNodesMap; };
    ::oox::drawingml::ShapePtr getShape( const ::rtl::OUString & id ) { return maShapeMap[ id ]; }
    ::oox::drawingml::ShapeIdMap& getShapeMap() { return maShapeMap; }

private:
    rtl::OUString                                                           maPath;
    rtl::OUString                                                           maLayoutPath;
    oox::vml::DrawingPtr                                                    mpDrawingPtr;
    com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage >    mxPage;
    oox::drawingml::ThemePtr                                                mpThemePtr;         // the theme that is used
    oox::drawingml::ClrSchemePtr                                            mpClrSchemePtr;     // the local color scheme (if any)
    oox::drawingml::ClrMapPtr                                               mpClrMapPtr;        // color mapping (if any)
    SlidePersistPtr                                                         mpMasterPagePtr;

    oox::drawingml::ShapePtr                                                maShapesPtr;
    oox::drawingml::FillPropertiesPtr                                       mpBackgroundPropertiesPtr;
    ::std::list< boost::shared_ptr< TimeNode > >                            maTimeNodeList;

    sal_Int32                                                               mnLayoutValueToken;
    sal_Bool                                                                mbMaster;
    sal_Bool                                                                mbNotes;

    oox::drawingml::TextListStylePtr                                        maDefaultTextStylePtr;
    oox::drawingml::TextListStylePtr                                        maTitleTextStylePtr;
    oox::drawingml::TextListStylePtr                                        maBodyTextStylePtr;
    oox::drawingml::TextListStylePtr                                        maNotesTextStylePtr;
    oox::drawingml::TextListStylePtr                                        maOtherTextStylePtr;

    std::map< ::rtl::OUString, ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > > maAnimNodesMap;
    std::map< ::rtl::OUString, ::oox::drawingml::ShapePtr > maShapeMap;
};

} }

#endif // OOX_POWERPOINT_SLIDEPERSIST_HXX
