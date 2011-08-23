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

#ifndef OOX_DRAWINGML_TEXTPARAGRAPHPROPERTIES_HXX
#define OOX_DRAWINGML_TEXTPARAGRAPHPROPERTIES_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/textcharacterproperties.hxx"
#include <com/sun/star/style/NumberingType.hpp>
#include "oox/drawingml/textfont.hxx"
#include "textspacing.hxx"
#include <boost/optional.hpp>

namespace com { namespace sun { namespace star {
    namespace graphic { class XGraphic; }
} } }

namespace oox { namespace drawingml {

class TextParagraphProperties;

typedef boost::shared_ptr< TextParagraphProperties > TextParagraphPropertiesPtr;

class BulletList
{
public:
    BulletList( );
    bool is() const;
    void apply( const BulletList& );
    void pushToPropMap( const ::oox::core::XmlFilterBase& rFilterBase, PropertyMap& rPropMap ) const;
    void setBulletChar( const ::rtl::OUString & sChar );
    void setStartAt( sal_Int32 nStartAt ){ mnStartAt <<= static_cast< sal_Int16 >( nStartAt ); }
    void setType( sal_Int32 nType );
    void setNone( );
    void setSuffixParenBoth();
    void setSuffixParenRight();
    void setSuffixPeriod();
    void setSuffixNone();
    void setSuffixMinusRight();
    void setBulletSize(sal_Int16 nSize);
    void setFontSize(sal_Int16 nSize);
    void setStyleName( const rtl::OUString& rStyleName ) { maStyleName <<= rStyleName; }
    void setGraphic( ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& rXGraphic );

    ::oox::drawingml::ColorPtr	maBulletColorPtr;
    ::com::sun::star::uno::Any	mbBulletColorFollowText;
    ::com::sun::star::uno::Any	mbBulletFontFollowText;
    ::oox::drawingml::TextFont	maBulletFont;
    ::com::sun::star::uno::Any	msBulletChar;
    ::com::sun::star::uno::Any	mnStartAt;
    ::com::sun::star::uno::Any	mnNumberingType;
    ::com::sun::star::uno::Any	msNumberingPrefix;
    ::com::sun::star::uno::Any	msNumberingSuffix;
    ::com::sun::star::uno::Any	mnSize;
    ::com::sun::star::uno::Any	mnFontSize;
    ::com::sun::star::uno::Any	maStyleName;
    ::com::sun::star::uno::Any  maGraphic;
    boost::optional< float >	maFollowFontSize;
};

class TextParagraphProperties
{
public:

    TextParagraphProperties();
    ~TextParagraphProperties();

    void                                setLevel( sal_Int16 nLevel ) { mnLevel = nLevel; }
    sal_Int16                           getLevel( ) const { return mnLevel; }
    PropertyMap&                        getTextParagraphPropertyMap() { return maTextParagraphPropertyMap; }
    BulletList&                         getBulletList() { return maBulletList; }
    TextCharacterProperties&            getTextCharacterProperties() { return maTextCharacterProperties; }
    const TextCharacterProperties&      getTextCharacterProperties() const { return maTextCharacterProperties; }

    TextSpacing&                        getParaTopMargin() { return maParaTopMargin; }
    TextSpacing&                        getParaBottomMargin() { return maParaBottomMargin; }
    boost::optional< sal_Int32 >&       getParaLeftMargin(){ return moParaLeftMargin; }
    boost::optional< sal_Int32 >&       getFirstLineIndentation(){ return moFirstLineIndentation; }

    void                                apply( const TextParagraphProperties& rSourceProps );
    void                                pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase,
                                            const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > & xPropSet,
                                                PropertyMap& rioBulletList, const BulletList* pMasterBuList, sal_Bool bApplyBulletList, float fFontSize ) const;

    /** Returns the largest character size of this paragraph. If possible the
        masterstyle should have been applied before, otherwise the character
        size can be zero and the default value is returned. */
    float                               getCharHeightPoints( float fDefault ) const;

#if OSL_DEBUG_LEVEL > 0
    void dump() { maTextParagraphPropertyMap.dump(); OSL_TRACE("character height: %f", maTextCharacterProperties.getCharHeightPoints(-1)); }
#endif

protected:

    TextCharacterProperties         maTextCharacterProperties;
    PropertyMap                     maTextParagraphPropertyMap;
    BulletList                      maBulletList;
    TextSpacing                     maParaTopMargin;
    TextSpacing                     maParaBottomMargin;
    boost::optional< sal_Int32 >    moParaLeftMargin;
    boost::optional< sal_Int32 >    moFirstLineIndentation;
    sal_Int16                       mnLevel;
};

} }

#endif  //  OOX_DRAWINGML_TEXTPARAGRAPHPROPERTIES_HXX
