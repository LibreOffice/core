/*************************************************************************
 *
 *  $RCSfile: property.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-02 11:08:18 $
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


#include <toolkit/helper/property.hxx>
#include <toolkit/helper/macros.hxx>

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <stdlib.h> // qsort/bsearch
#include <tools/debug.hxx>

#ifndef _COM_SUN_STAR_AWT_FONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_CHARSET_HPP_
#include <com/sun/star/awt/CharSet.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWIDTH_HPP_
#include <com/sun/star/awt/FontWidth.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTTYPE_HPP_
#include <com/sun/star/awt/FontType.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSTRIKEOUT_HPP_
#include <com/sun/star/awt/FontStrikeout.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTPITCH_HPP_
#include <com/sun/star/awt/FontPitch.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#include <toolkit/helper/property.hxx>

struct ImplPropertyInfo
{
    ::rtl::OUString                 aName;
    sal_uInt16                      nPropId;
    ::com::sun::star::uno::Type     aType;
    sal_Int16                       nAttribs;
    sal_Bool                        bDependsOnOthers;   // eg. VALUE depends on MIN/MAX and must be set after MIN/MAX.

    ImplPropertyInfo()
     {
         nPropId = 0;
        nAttribs = 0;
           bDependsOnOthers = sal_False;
     }

    ImplPropertyInfo( const sal_Unicode* pName, sal_uInt16 nId, const ::com::sun::star::uno::Type& rType,
                        sal_Int16 nAttrs, sal_Bool bDepends = sal_False )
     : aName( pName )
     {
         nPropId = nId;
        aType = rType;
        nAttribs = nAttrs;
           bDependsOnOthers = bDepends;
     }

};

ImplPropertyInfo* ImplGetPropertyInfos( sal_uInt16& rElementCount )
{
    static ImplPropertyInfo* pPropertyInfos = NULL;
    static sal_uInt16 nElements = 0;
    if( !pPropertyInfos )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pPropertyInfos )
        {
            static ImplPropertyInfo __FAR_DATA aImplPropertyInfos [] =
            {
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "Align" ),              BASEPROPERTY_ALIGN,             ::getCppuType((const sal_Int16*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT|::com::sun::star::beans::PropertyAttribute::MAYBEVOID ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "Autocomplete" ),       BASEPROPERTY_AUTOCOMPLETE,      ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "AutoToggle" ),         BASEPROPERTY_AUTOTOGGLE,        ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "BackgroundColor" ),    BASEPROPERTY_BACKGROUNDCOLOR,   ::getCppuType((const sal_Int32*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT|::com::sun::star::beans::PropertyAttribute::MAYBEVOID ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "Border" ),             BASEPROPERTY_BORDER,            ::getCppuType((const sal_Int16*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT|::com::sun::star::beans::PropertyAttribute::MAYBEVOID ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "CurrencySymbol" ),     BASEPROPERTY_CURRENCYSYMBOL,    ::getCppuType((const ::rtl::OUString*)0),       ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "Date" ),               BASEPROPERTY_DATE,              ::getCppuType((const sal_Int32*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT|::com::sun::star::beans::PropertyAttribute::MAYBEVOID, sal_True ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "DateFormat" ),         BASEPROPERTY_EXTDATEFORMAT,     ::getCppuType((const sal_Int16*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "DateMax" ),            BASEPROPERTY_DATEMAX,           ::getCppuType((const sal_Int32*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "DateMin" ),            BASEPROPERTY_DATEMIN,           ::getCppuType((const sal_Int32*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "DateShowCentury" ),    BASEPROPERTY_DATESHOWCENTURY,   ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT|::com::sun::star::beans::PropertyAttribute::MAYBEVOID ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "DecimalAccuracy" ),    BASEPROPERTY_DECIMALACCURACY,   ::getCppuType((const sal_Int16*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "DefaultButton" ),      BASEPROPERTY_DEFAULTBUTTON,     ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "DefaultControl" ),     BASEPROPERTY_DEFAULTCONTROL,    ::getCppuType((const ::rtl::OUString*)0),       ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "Dropdown" ),           BASEPROPERTY_DROPDOWN,          ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "EchoChar" ),           BASEPROPERTY_ECHOCHAR,          ::getCppuType((const sal_Int16*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "EditMask" ),           BASEPROPERTY_EDITMASK,          ::getCppuType((const ::rtl::OUString*)0),       ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "EffectiveDefault" ),   BASEPROPERTY_EFFECTIVE_DEFAULT, ::getCppuType((const ::com::sun::star::uno::Any*)0),            ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT|::com::sun::star::beans::PropertyAttribute::MAYBEVOID ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "EffectiveMax" ),       BASEPROPERTY_EFFECTIVE_MAX,     ::getCppuType((const Double*)0),        ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT|::com::sun::star::beans::PropertyAttribute::MAYBEVOID ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "EffectiveMin" ),       BASEPROPERTY_EFFECTIVE_MIN,     ::getCppuType((const Double*)0),        ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT|::com::sun::star::beans::PropertyAttribute::MAYBEVOID ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "EffectiveValue" ),     BASEPROPERTY_EFFECTIVE_VALUE,   ::getCppuType((const ::com::sun::star::uno::Any*)0),            ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT|::com::sun::star::beans::PropertyAttribute::MAYBEVOID, sal_True ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "Enabled" ),            BASEPROPERTY_ENABLED,           ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FillColor" ),          BASEPROPERTY_FILLCOLOR,         ::getCppuType((const sal_Int32*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT|::com::sun::star::beans::PropertyAttribute::MAYBEVOID ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FocusSelectionHide" ), BASEPROPERTY_FOCUSSELECTIONHIDE,::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FontDescriptor" ),     BASEPROPERTY_FONTDESCRIPTOR,    ::getCppuType((const ::com::sun::star::awt::FontDescriptor*)0), ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),

            // Teile des ::com::sun::star::awt::FontDescriptor
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FontName" ),           BASEPROPERTY_FONTDESCRIPTORPART_NAME,           ::getCppuType((const ::rtl::OUString*)0),   ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT /*, PropertyState_AMBIGUOUS_VALUE */ ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FontStyleName" ),      BASEPROPERTY_FONTDESCRIPTORPART_STYLENAME,      ::getCppuType((const ::rtl::OUString*)0),   ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT /*, PropertyState_AMBIGUOUS_VALUE */ ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FontFamily" ),         BASEPROPERTY_FONTDESCRIPTORPART_FAMILY,         ::getCppuType((const sal_Int16*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT /*, PropertyState_AMBIGUOUS_VALUE */ ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FontCharset" ),        BASEPROPERTY_FONTDESCRIPTORPART_CHARSET,        ::getCppuType((const sal_Int16*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT /*, PropertyState_AMBIGUOUS_VALUE */ ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FontHeight" ),         BASEPROPERTY_FONTDESCRIPTORPART_HEIGHT,         ::getCppuType((const Float*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT /*, PropertyState_AMBIGUOUS_VALUE */ ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FontWidth" ),          BASEPROPERTY_FONTDESCRIPTORPART_WIDTH,          ::getCppuType((const sal_Int16*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT /*, PropertyState_AMBIGUOUS_VALUE */ ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FontPitch" ),          BASEPROPERTY_FONTDESCRIPTORPART_PITCH,          ::getCppuType((const sal_Int16*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT /*, PropertyState_AMBIGUOUS_VALUE */ ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FontWeight" ),         BASEPROPERTY_FONTDESCRIPTORPART_WEIGHT,         ::getCppuType((const Float*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT /*, PropertyState_AMBIGUOUS_VALUE */ ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FontCharWidth" ),      BASEPROPERTY_FONTDESCRIPTORPART_CHARWIDTH,      ::getCppuType((const Float*)0),     ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT /*, PropertyState_AMBIGUOUS_VALUE */ ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FontOrientation" ),    BASEPROPERTY_FONTDESCRIPTORPART_ORIENTATION,    ::getCppuType((const Float*)0),     ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT /*, PropertyState_AMBIGUOUS_VALUE */ ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FontSlant" ),          BASEPROPERTY_FONTDESCRIPTORPART_SLANT,          ::getCppuType((const sal_Int16*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT /*, PropertyState_AMBIGUOUS_VALUE */ ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FontUnderline" ),      BASEPROPERTY_FONTDESCRIPTORPART_UNDERLINE,      ::getCppuType((const sal_Int16*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT /*, PropertyState_AMBIGUOUS_VALUE */ ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FontStrikeout" ),      BASEPROPERTY_FONTDESCRIPTORPART_STRIKEOUT,      ::getCppuType((const sal_Int16*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT /*, PropertyState_AMBIGUOUS_VALUE */ ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FontKerning" ),        BASEPROPERTY_FONTDESCRIPTORPART_KERNING,        ::getBooleanCppuType(),     ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT /*, PropertyState_AMBIGUOUS_VALUE */ ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FontWordLineMode" ),   BASEPROPERTY_FONTDESCRIPTORPART_WORDLINEMODE,   ::getBooleanCppuType(),     ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT /*, PropertyState_AMBIGUOUS_VALUE */ ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FontType" ),           BASEPROPERTY_FONTDESCRIPTORPART_TYPE,           ::getCppuType((const sal_Int16*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT /*, PropertyState_AMBIGUOUS_VALUE */ ),

            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FormatKey" ),          BASEPROPERTY_FORMATKEY,         ::getCppuType((const sal_Int32*)0),                 ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEVOID|::com::sun::star::beans::PropertyAttribute::TRANSIENT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "FormatsSupplier" ),    BASEPROPERTY_FORMATSSUPPLIER,   ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >*)0),    ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEVOID|::com::sun::star::beans::PropertyAttribute::TRANSIENT ),

            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "HelpURL" ),            BASEPROPERTY_HELPURL,           ::getCppuType((const ::rtl::OUString*)0),       ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "HScroll" ),            BASEPROPERTY_HSCROLL,           ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "HardLineBreaks" ),     BASEPROPERTY_HARDLINEBREAKS,    ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "ImageURL" ),           BASEPROPERTY_IMAGEURL,          ::getCppuType((const ::rtl::OUString*)0),       ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "Label" ),              BASEPROPERTY_LABEL,             ::getCppuType((const ::rtl::OUString*)0),       ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "LineColor" ),          BASEPROPERTY_LINECOLOR,         ::getCppuType((const sal_Int32*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT|::com::sun::star::beans::PropertyAttribute::MAYBEVOID ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "LineCount" ),          BASEPROPERTY_LINECOUNT,         ::getCppuType((const sal_Int16*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "LiteralMask" ),        BASEPROPERTY_LITERALMASK,       ::getCppuType((const ::rtl::OUString*)0),       ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "MaxTextLen" ),         BASEPROPERTY_MAXTEXTLEN,        ::getCppuType((const sal_Int16*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "MultiLine" ),          BASEPROPERTY_MULTILINE,         ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "MultiSelection" ),     BASEPROPERTY_MULTISELECTION,    ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "PrependCurrencySymbol" ),  BASEPROPERTY_CURSYM_POSITION,   ::getBooleanCppuType(),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "Printable" ),          BASEPROPERTY_PRINTABLE,         ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "ReadOnly" ),           BASEPROPERTY_READONLY,          ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "SelectedItems" ),      BASEPROPERTY_SELECTEDITEMS,     ::getCppuType((::com::sun::star::uno::Sequence<sal_Int16>*)0),      ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "ShowThousandsSeparator" ), BASEPROPERTY_NUMSHOWTHOUSANDSEP,::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "Spin" ),               BASEPROPERTY_SPIN,              ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "State" ),              BASEPROPERTY_STATE,             ::getCppuType((const sal_Int16*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "StrictFormat" ),       BASEPROPERTY_STRICTFORMAT,      ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "StringItemList" ),     BASEPROPERTY_STRINGITEMLIST,    ::getCppuType((::com::sun::star::uno::Sequence< ::rtl::OUString >*)0),      ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "Tabstop" ),            BASEPROPERTY_TABSTOP,           ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT|::com::sun::star::beans::PropertyAttribute::MAYBEVOID ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "Text" ),               BASEPROPERTY_TEXT,              ::getCppuType((const ::rtl::OUString*)0),       ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "TextColor" ),          BASEPROPERTY_TEXTCOLOR,         ::getCppuType((const sal_Int32*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT|::com::sun::star::beans::PropertyAttribute::MAYBEVOID ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "Time" ),               BASEPROPERTY_TIME,              ::getCppuType((const sal_Int32*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT|::com::sun::star::beans::PropertyAttribute::MAYBEVOID, sal_True ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "TimeFormat" ),         BASEPROPERTY_EXTTIMEFORMAT,     ::getCppuType((const sal_Int16*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "TimeMax" ),            BASEPROPERTY_TIMEMAX,           ::getCppuType((const sal_Int32*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "TimeMin" ),            BASEPROPERTY_TIMEMIN,           ::getCppuType((const sal_Int32*)0),     ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "TreatAsNumber" ),      BASEPROPERTY_TREATASNUMBER,     ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT|::com::sun::star::beans::PropertyAttribute::TRANSIENT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "TriState" ),           BASEPROPERTY_TRISTATE,          ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "VScroll" ),            BASEPROPERTY_VSCROLL,           ::getBooleanCppuType(),         ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "Value" ),              BASEPROPERTY_VALUE_DOUBLE,      ::getCppuType((const Double*)0),        ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT|::com::sun::star::beans::PropertyAttribute::MAYBEVOID, sal_True ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "ValueMax" ),           BASEPROPERTY_VALUEMAX_DOUBLE,   ::getCppuType((const Double*)0),        ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "ValueMin" ),           BASEPROPERTY_VALUEMIN_DOUBLE,   ::getCppuType((const Double*)0),        ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            ImplPropertyInfo( ::rtl::OUString::createFromAscii( "ValueStep" ),          BASEPROPERTY_VALUESTEP_DOUBLE,  ::getCppuType((const Double*)0),        ::com::sun::star::beans::PropertyAttribute::BOUND|::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT ),
            };
            pPropertyInfos = aImplPropertyInfos;
            nElements = sizeof( aImplPropertyInfos ) / sizeof( ImplPropertyInfo );
        }
    }
    rElementCount = nElements;
    return pPropertyInfos;
}

static int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC ) && defined( OS2 )
_Optlink
#endif
     ImplPropertyInfoCompare( const void* pFirst, const void* pSecond)
{
    return ((ImplPropertyInfo*)pFirst)->aName.compareTo( ((ImplPropertyInfo*)pSecond)->aName );
}

void ImplAssertValidPropertyArray()
{
    static sal_Bool bSorted = sal_False;
    if( !bSorted )
    {
        sal_uInt16 nElements;
        ImplPropertyInfo* pInfos = ImplGetPropertyInfos( nElements );
        qsort( pInfos, nElements, sizeof( ImplPropertyInfo ), ImplPropertyInfoCompare );
        bSorted = sal_True;
    }
}

sal_uInt16 GetPropertyId( const ::rtl::OUString& rPropertyName )
{
    ImplAssertValidPropertyArray();

    ImplPropertyInfo aSearch;
    aSearch.aName = rPropertyName;

    sal_uInt16 nElements;
    ImplPropertyInfo* pInfos = ImplGetPropertyInfos( nElements );
    ImplPropertyInfo* pInf = (ImplPropertyInfo*)
                                bsearch( &aSearch, pInfos, nElements, sizeof( ImplPropertyInfo ), ImplPropertyInfoCompare );

    return pInf ? pInf->nPropId: 0;
}

const ImplPropertyInfo* ImplGetImplPropertyInfo( sal_uInt16 nPropertyId )
{
    ImplAssertValidPropertyArray();

    const ImplPropertyInfo* pImplPropertyInfo = NULL;

    sal_uInt16 nElements;
    ImplPropertyInfo* pInfos = ImplGetPropertyInfos( nElements );
    for ( sal_uInt16 n = nElements; n && !pImplPropertyInfo; )
    {
        if ( pInfos[--n].nPropId == nPropertyId )
            pImplPropertyInfo = &pInfos[n];
    }

    return pImplPropertyInfo;
}

sal_uInt16 GetPropertyOrderNr( sal_uInt16 nPropertyId )
{
    ImplAssertValidPropertyArray();

    sal_uInt16 nElements;
    ImplPropertyInfo* pInfos = ImplGetPropertyInfos( nElements );
    for ( sal_uInt16 n = nElements; n; )
    {
        if ( pInfos[--n].nPropId == nPropertyId )
            return n;
    }
    return 0xFFFF;
}

const ::rtl::OUString& GetPropertyName( sal_uInt16 nPropertyId )
{
    const ImplPropertyInfo* pImplPropertyInfo = ImplGetImplPropertyInfo( nPropertyId );
    DBG_ASSERT( pImplPropertyInfo, "Invalid PropertyId!" );
    return pImplPropertyInfo->aName;
}

const ::com::sun::star::uno::Type* GetPropertyType( sal_uInt16 nPropertyId )
{
    const ImplPropertyInfo* pImplPropertyInfo = ImplGetImplPropertyInfo( nPropertyId );
    DBG_ASSERT( pImplPropertyInfo, "Invalid PropertyId!" );
    return pImplPropertyInfo ? &pImplPropertyInfo->aType : NULL;
}

sal_Int16 GetPropertyAttribs( sal_uInt16 nPropertyId )
{
    const ImplPropertyInfo* pImplPropertyInfo = ImplGetImplPropertyInfo( nPropertyId );
    DBG_ASSERT( pImplPropertyInfo, "Invalid PropertyId!" );
    return pImplPropertyInfo ? pImplPropertyInfo->nAttribs : NULL;
}

sal_Bool DoesDependOnOthers( sal_uInt16 nPropertyId )
{
    const ImplPropertyInfo* pImplPropertyInfo = ImplGetImplPropertyInfo( nPropertyId );
    DBG_ASSERT( pImplPropertyInfo, "Invalid PropertyId!" );
    return pImplPropertyInfo ? pImplPropertyInfo->bDependsOnOthers : sal_False;
}

sal_Bool CompareProperties( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 )
{
    return ::comphelper::compare( r1, r2 );
}



