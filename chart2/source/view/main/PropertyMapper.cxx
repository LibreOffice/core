/*************************************************************************
 *
 *  $RCSfile: PropertyMapper.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:34 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "PropertyMapper.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;

//static
void PropertyMapper::setMappedProperties(
          const uno::Reference< beans::XPropertySet >& xTarget
        , const uno::Reference< beans::XPropertySet >& xSource
        , const tPropertyNameMap& rMap )
{
    if( !xTarget.is() || !xSource.is() )
        return;

    uno::Reference< beans::XMultiPropertySet > xMultiProp( xTarget, uno::UNO_QUERY );
    if( xMultiProp.is() )
    {
        tNameSequence aNames;
        tAnySequence  aValues;
        getMultiPropertyLists(aNames, aValues, xSource, rMap );
        try
        {
            xMultiProp->setPropertyValues( aNames,aValues );
        }
        catch( uno::Exception& e )
        {
            e;
        }
    }
    else
    {
        tPropertyNameMap::const_iterator aIt( rMap.begin() );
        tPropertyNameMap::const_iterator aEnd( rMap.end() );
        for( ; aIt != aEnd; ++aIt )
        {
            rtl::OUString aTarget = aIt->first;
            rtl::OUString aSource = aIt->second;
            try
            {
                uno::Any aAny( xSource->getPropertyValue(aSource) );
                xTarget->setPropertyValue( aTarget, aAny );
            }
            catch( uno::Exception& e )
            {
                e;
            }
        }
    }
}

void PropertyMapper::getValueMap(
                  tPropertyNameValueMap& rValueMap
                , const tPropertyNameMap& rNameMap
                , const uno::Reference< beans::XPropertySet >& xSourceProp
                )
{
    tPropertyNameMap::const_iterator aIt( rNameMap.begin() );
    tPropertyNameMap::const_iterator aEnd( rNameMap.end() );

    for( ; aIt != aEnd; ++aIt )
    {
        rtl::OUString aTarget = aIt->first;
        rtl::OUString aSource = aIt->second;
        try
        {
            uno::Any aAny( xSourceProp->getPropertyValue(aSource) );
            if( aAny.hasValue() )
                rValueMap.insert( tPropertyNameValueMap::value_type( aTarget, aAny ) );
        }
        catch( uno::Exception& e )
        {
            e;
        }
    }
}

void PropertyMapper::getMultiPropertyLists(
                  tNameSequence& rNames
                , tAnySequence&  rValues
                , const uno::Reference< beans::XPropertySet >& xSourceProp
                , const tPropertyNameMap& rNameMap
                )
{
    tPropertyNameValueMap aValueMap;
    getValueMap( aValueMap, rNameMap, xSourceProp );
    getMultiPropertyListsFromValueMap( rNames, rValues, aValueMap );
}

void PropertyMapper::getMultiPropertyListsFromValueMap(
                  tNameSequence& rNames
                , tAnySequence&  rValues
                , const tPropertyNameValueMap& rValueMap
                )
{
    sal_Int32 nPropertyCount = rValueMap.size();
    rNames.realloc(nPropertyCount);
    rValues.realloc(nPropertyCount);

    //fill sequences
    tPropertyNameValueMap::const_iterator aValueIt(  rValueMap.begin() );
    tPropertyNameValueMap::const_iterator aValueEnd( rValueMap.end()   );
    sal_Int32 nN=0;
    for( ; aValueIt != aValueEnd; ++aValueIt )
    {
        const uno::Any& rAny = aValueIt->second;
        if( rAny.hasValue() )
        {
            //do not set empty anys because of performance (otherwise SdrAttrObj::ItemChange will take much longer)
            rNames[nN]  = aValueIt->first;
            rValues[nN] = rAny;
            ++nN;
        }
    }
    //reduce to real property count
    rNames.realloc(nN);
    rValues.realloc(nN);
}

uno::Any* PropertyMapper::getValuePointer( tAnySequence& rPropValues
                         , const tNameSequence& rPropNames
                         , const rtl::OUString& rPropName )
{
    sal_Int32 nCount = rPropNames.getLength();
    for( sal_Int32 nN = 0; nN < nCount; nN++ )
    {
        if(rPropNames[nN].equals(rPropName))
            return &rPropValues[nN];
    }
    return NULL;
}

/*
//set some properties from service style::CharacterProperties:
//-------- tabpage: Zeichen -----------
//Schriftart z.B. Albany            UNO_NAME_EDIT_CHAR_FONTNAME == UNO_NAME_EDIT_CHAR_FONTSTYLENAME    //UNO_NAME_CHAR_FONT
//Schriftschnitt z.B. kursiv        UNO_NAME_EDIT_CHAR_POSTURE    UNO_NAME_CHAR_POSTURE awt::FontSlant NONE OBLIQUE ITALIC DONTKNOW REVERSE_OBLIQUE REVERSE_ITALIC
//Schriftgrad (Punktgrösse z.B. 12) UNO_NAME_EDIT_CHAR_HEIGHT == UNO_NAME_CHAR_HEIGHT
        //? UNO_NAME_EDIT_CHAR_WEIGHT == UNO_NAME_CHAR_WEIGHT
//Sprache                           UNO_NAME_EDIT_CHAR_LOCALE lang::Locale

//-------- tabpage: Schrifteffekt -----------
//Unterstreichung                   UNO_NAME_CHAR_UNDERLINE sal_Int16 awt::FontUnderline_NONE _SINGLE _DOUBLE _DOTTED _DONTKNOW _DASH ...
//Unterstreichung-farbe             ??? 'CharUnderlineColor' + CharUnderlineHasColor
//Durchstreichung z.B. doppelt      "CharStrikeout" sal_Int16 awt::FontStrikeout_NONE _SINGLE _DOUBLE ...
//wortweise-Durchstreichung ja/nein "CharWordMode" bool
//Schriftfarbe                      UNO_NAME_EDIT_CHAR_COLOR sal_Int32      UNO_NAME_CHAR_COLOR
//ReliefArt ohne/erhaben/tief       "CharRelief" sal_Int16 text::FontRelief_NONE FontRelief_EMBOSSED FontRelief_ENGRAVED
//Kontur                            "CharContoured" bool
//Schatten                          UNO_NAME_CHAR_SHADOWED bool
*/

//static
const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForCharacterProperties()
{
    //shape property -- chart model object property
    static tMakePropertyNameMap m_aShapePropertyMapForCharacterProperties =
        tMakePropertyNameMap
        ( C2U( "CharFontName" ), C2U("CharFontName") )
        ( C2U( "CharFontStyleName" ), C2U("CharFontStyleName") )
        ( C2U( "CharFontFamily" ), C2U("CharFontFamily") )
        ( C2U( "CharFontCharSet" ), C2U("CharFontCharSet") )
        ( C2U( "CharFontPitch" ), C2U("CharFontPitch") )
        ( C2U( "CharColor" ), C2U("CharColor") )
//        ( C2U( "CharBackColor" ), C2U("TextBackgroundColor") )
        ( C2U( "CharEscapement" ), C2U("CharEscapement") )
        ( C2U( "CharHeight" ), C2U("CharHeight") )
        ( C2U( "CharUnderline" ), C2U("CharUnderline") )
        ( C2U( "CharUnderlineColor" ), C2U("CharUnderlineColor") )
        ( C2U( "CharUnderlineHasColor" ), C2U("CharUnderlineHasColor") )
        ( C2U( "CharWeight" ), C2U("CharWeight") )
        ( C2U( "CharPosture" ), C2U("CharPosture") )
        ( C2U( "CharKerning" ), C2U("CharKerning") )
//        ( C2U( "CharCaseMap" ), C2U("CaseMapping") )
//        ( C2U( "CharRotation" ), C2U("Rotation") ) --> additional feature ...
//         ( C2U( "CharScaleWidth" ), C2U("CharScaleWidth") )
//        ( C2U( "CharEscapementHeight" ), C2U("EscapementHeight") ) -> assertion
///////        ( C2U( "CharCrossedOut" ), C2U("CharCrossedOut") ) //setting this explicitly somehow conflicts with CharStrikeout
        ( C2U( "CharStrikeout" ), C2U("CharStrikeout") )
        ( C2U( "CharWordMode" ), C2U("CharWordMode") )
//        ( C2U( "CharFlash" ), C2U("Flashing") )
        ( C2U( "CharLocale" ), C2U("CharLocale") )
        ( C2U( "CharShadowed" ), C2U("CharShadowed") )
        ( C2U( "CharContoured" ), C2U("CharContoured") )
        ( C2U( "CharRelief" ), C2U("CharRelief") )
        ( C2U( "CharEmphasis" ), C2U("CharEmphasis") )//the service style::CharacterProperties  describes a property called 'CharEmphasize' wich is nowhere implemented
//        ( C2U( "RubyText" ), C2U("RubyText") )
//        ( C2U( "RubyAdjust" ), C2U("RubyAdjust") )
//        ( C2U( "RubyCharStyleName" ), C2U("RubyStyleName") )
//        ( C2U( "RubyIsAbove" ), C2U("RubyIsAbove") )
//        ( C2U( "CharNoHyphenation" ), C2U("InhibitHyphenation") )
        ( C2U( "CharFontStyleNameAsian" ), C2U("CharFontNameAsian") )
        ( C2U( "CharFontStyleNameAsian" ), C2U("CharFontStyleNameAsian") )
        ( C2U( "CharFontFamilyAsian" ), C2U("CharFontFamilyAsian") )
        ( C2U( "CharFontCharSetAsian" ), C2U("CharFontCharSetAsian") )
        ( C2U( "CharFontPitchAsian" ), C2U("CharFontPitchAsian") )
        ( C2U( "CharHeightAsian" ), C2U("CharHeightAsian") )
        ( C2U( "CharWeightAsian" ), C2U("CharWeightAsian") )
        ( C2U( "CharPostureAsian" ), C2U("CharPostureAsian") )
        ( C2U( "CharLocaleAsian" ), C2U("CharLocaleAsian") )

        ( C2U( "CharFontStyleNameComplex" ), C2U("CharFontNameComplex") )
        ( C2U( "CharFontStyleNameComplex" ), C2U("CharFontStyleNameComplex") )
        ( C2U( "CharFontFamilyComplex" ), C2U("CharFontFamilyComplex") )
        ( C2U( "CharFontCharSetComplex" ), C2U("CharFontCharSetComplex") )
        ( C2U( "CharFontPitchComplex" ), C2U("CharFontPitchComplex") )
        ( C2U( "CharHeightComplex" ), C2U("CharHeightComplex") )
        ( C2U( "CharWeightComplex" ), C2U("CharWeightComplex") )
        ( C2U( "CharPostureComplex" ), C2U("CharPostureComplex") )
        ( C2U( "CharLocaleComplex" ), C2U("CharLocaleComplex") )
        ;
    return m_aShapePropertyMapForCharacterProperties;
}

//static
const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForParagraphProperties()
{
    //shape property -- chart model object property
    static tMakePropertyNameMap m_aShapePropertyMapForParagraphProperties =
        tMakePropertyNameMap
        ( C2U( "ParaAdjust" ), C2U("ParaAdjust") )
        ( C2U( "ParaLastLineAdjust" ), C2U("ParaLastLineAdjust") )
        ( C2U( "ParaLeftMargin" ), C2U("ParaLeftMargin") )
        ( C2U( "ParaRightMargin" ), C2U("ParaRightMargin") )
        ( C2U( "ParaTopMargin" ), C2U("ParaTopMargin") )
        ( C2U( "ParaBottomMargin" ), C2U("ParaBottomMargin") )
        ( C2U( "ParaIsHyphenation" ), C2U("ParaIsHyphenation") )
        ;
    return m_aShapePropertyMapForParagraphProperties;
}

//static
const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForFillProperties()
{
    //shape property -- chart model object property
    static tMakePropertyNameMap m_aShapePropertyMapForFillProperties =
        tMakePropertyNameMap
        ( C2U( "FillStyle" ), C2U("FillStyle") )
        ( C2U( "FillColor" ), C2U("FillColor") )
        ( C2U( "FillTransparence" ), C2U("FillTransparence") )
        ( C2U( "FillGradient" ), C2U("FillGradient") )
        ( C2U( "FillHatch" ), C2U("FillHatch") )
//        ( C2U( "FillTransparenceGradientName" ), C2U("TransparencyStyle") ) //@todo this property name seems to be wrong in chart model
//        ( C2U( "FillTransparenceGradient" ), C2U("TransparencyGradient") ) //@todo this property name seems to be wrong in chart model
        ;
    return m_aShapePropertyMapForFillProperties;
}

//static
const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForLineProperties()
{
    //shape property -- chart model object property
    static tMakePropertyNameMap m_aShapePropertyMapForLineProperties =
        tMakePropertyNameMap
        ( C2U( "LineStyle" ), C2U("LineStyle") )
        ( C2U( "LineWidth" ), C2U("LineWidth") )
        ( C2U( "LineDash" ), C2U("LineDash") )
        ( C2U( "LineColor" ), C2U("LineColor") )
        ( C2U( "LineTransparence" ), C2U("LineTransparence") )
        ( C2U( "LineJoint" ), C2U("LineJoint") )
        ;
    return m_aShapePropertyMapForLineProperties;
}

//static
void PropertyMapper::setMultiProperties(
                  const tNameSequence& rNames
                , const tAnySequence&  rValues
                , const uno::Reference< drawing::XShape >& xTarget )
{
    try
    {
        uno::Reference< beans::XMultiPropertySet > xShapeMultiProp( xTarget, uno::UNO_QUERY );
        if( xShapeMultiProp.is() )
        {
            xShapeMultiProp->setPropertyValues( rNames,rValues );
        }
        else
        {
            //@todo: if no multipropertyset is available try the unperformant normal XPropertySet
        }
    }
    catch( uno::Exception& e )
    {
        e;
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
