/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlnumfi.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:34:14 $
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

#ifndef _XMLOFF_XMLNUMFI_HXX
#define _XMLOFF_XMLNUMFI_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _XMLOFF_XMLSTYLE_HXX
#include <xmloff/xmlstyle.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif
#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

#define XML_NUMBERSTYLES "NumberStyles"

enum SvXMLStylesTokens
{
    XML_TOK_STYLES_NUMBER_STYLE,
    XML_TOK_STYLES_CURRENCY_STYLE,
    XML_TOK_STYLES_PERCENTAGE_STYLE,
    XML_TOK_STYLES_DATE_STYLE,
    XML_TOK_STYLES_TIME_STYLE,
    XML_TOK_STYLES_BOOLEAN_STYLE,
    XML_TOK_STYLES_TEXT_STYLE
};

enum SvXMLDateElementAttributes
{
    XML_DEA_NONE,
    XML_DEA_ANY,
    XML_DEA_SHORT,
    XML_DEA_LONG,
    XML_DEA_TEXTSHORT,
    XML_DEA_TEXTLONG
};

class Color;
class SvXMLNumImpData;
class SvXMLImport;
class SvXMLStyleContext;
class SvXMLStylesContext;
struct SvXMLNumberInfo;
class SvNumberFormatter;
class SvtSysLocale;

//  use SvXMLNumFmtHelper in the context for <office:styles> to create
//  child contexts for data styles

class SvXMLNumFmtHelper
{
    SvXMLNumImpData* pData;

    // #110680#
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxServiceFactory;

public:
    // #110680#
    //SvXMLNumFmtHelper( const ::com::sun::star::uno::Reference<
    //      ::com::sun::star::util::XNumberFormatsSupplier >& rSupp );
    //SvXMLNumFmtHelper( SvNumberFormatter* pNumberFormatter );
    SvXMLNumFmtHelper(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& rSupp,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory );

    SvXMLNumFmtHelper(
        SvNumberFormatter* pNumberFormatter,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory );

    ~SvXMLNumFmtHelper();

    SvXMLStyleContext*  CreateChildContext( SvXMLImport& rImport,
                USHORT nPrefix, const rtl::OUString& rLocalName,
                const ::com::sun::star::uno::Reference<
                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                    SvXMLStylesContext& rStyles);

    SvXMLNumImpData* getData() { return pData; }

    const SvXMLTokenMap&    GetStylesElemTokenMap();

//  sal_uInt32  GetKeyForName( const rtl::OUString& rName );
};

//  SvXMLNumFmtDefaults is used in import and export

class SvXMLNumFmtDefaults
{
public:
    static sal_Bool IsSystemLongDay( const SvtSysLocale& rSysLoc, BOOL bLong );
    static sal_Bool IsSystemLongMonth( const SvtSysLocale& rSysLoc, BOOL bLong );
    static sal_Bool IsSystemTextualMonth( const SvtSysLocale& rSysLoc, BOOL bLong );
    static sal_Bool IsSystemLongYear( const SvtSysLocale& rSysLoc, BOOL bLong );
    static sal_Bool IsSystemLongEra( const SvtSysLocale& rSysLoc, BOOL bLong );
    static sal_Bool IsSystemLongDayOfWeek( const SvtSysLocale& rSysLoc, BOOL bLong );

    // return value is NfIndexTableOffset
    static sal_uInt16 GetDefaultDateFormat( SvXMLDateElementAttributes eDOW,
                SvXMLDateElementAttributes eDay, SvXMLDateElementAttributes eMonth,
                SvXMLDateElementAttributes eYear, SvXMLDateElementAttributes eHours,
                SvXMLDateElementAttributes eMins, SvXMLDateElementAttributes eSecs,
                sal_Bool bSystem );
};

struct MyCondition
{
    rtl::OUString   sCondition;
    rtl::OUString   sMapName;
};

class XMLOFF_DLLPUBLIC SvXMLNumFormatContext : public SvXMLStyleContext
{
    SvXMLNumImpData*    pData;
    SvXMLStylesContext*             pStyles;
    std::vector <MyCondition>   aMyConditions;
    sal_uInt16          nType;
    sal_Int32           nKey;
//  rtl::OUString       sFormatName;
    rtl::OUString       sFormatTitle;
//  rtl::OUString       sMapName;
    rtl::OUString       sCalendar;
    rtl::OUString       sFormatString;
    LanguageType        nFormatLang;
    com::sun::star::lang::Locale    aLocale;
    sal_Bool            bAutoOrder;
    sal_Bool            bFromSystem;
    sal_Bool            bTruncate;
    sal_Bool            bAutoDec;       // set in AddNumber
    sal_Bool            bAutoInt;       // set in AddNumber
    sal_Bool            bHasExtraText;
    rtl::OUStringBuffer aFormatCode;
    rtl::OUStringBuffer aConditions;
    sal_Bool            bHasLongDoW;
    sal_Bool            bHasEra;
    sal_Bool            bHasDateTime;
    sal_Bool            bRemoveAfterUse;

    //  contained date elements, used to recognize default date formats
    SvXMLDateElementAttributes  eDateDOW;
    SvXMLDateElementAttributes  eDateDay;
    SvXMLDateElementAttributes  eDateMonth;
    SvXMLDateElementAttributes  eDateYear;
    SvXMLDateElementAttributes  eDateHours;
    SvXMLDateElementAttributes  eDateMins;
    SvXMLDateElementAttributes  eDateSecs;
    sal_Bool                    bDateNoDefault;

    SAL_DLLPRIVATE sal_Int32 PrivateGetKey();

public:
                SvXMLNumFormatContext( SvXMLImport& rImport, USHORT nPrfx,
                                    const rtl::OUString& rLName,
                                    SvXMLNumImpData* pNewData, sal_uInt16 nNewType,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                    SvXMLStylesContext& rStyles );
                SvXMLNumFormatContext( SvXMLImport& rImport, USHORT nPrfx,
                                    const rtl::OUString& rLName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                    const sal_Int32 nKey,
                                    SvXMLStylesContext& rStyles );
    virtual     ~SvXMLNumFormatContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                    const rtl::OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void CreateAndInsert(sal_Bool bOverwrite);
    virtual void Finish(sal_Bool bOverwrite);

    SvXMLNumImpData* GetData() const                { return pData; }
    sal_Int32 GetKey();
    sal_Int32 CreateAndInsert( SvNumberFormatter* pFormatter );
    sal_Int32 CreateAndInsert( com::sun::star::uno::Reference< com::sun::star::util::XNumberFormatsSupplier >& xFormatsSupplier );
    sal_uInt16 GetType() const                      { return nType; }   // SvXMLStylesTokens

    sal_Bool IsFromSystem() const                   { return bFromSystem; }
    sal_Bool HasLongDoW() const                     { return bHasLongDoW; }
    void SetHasLongDoW(sal_Bool bSet)               { bHasLongDoW = bSet; }
    sal_Bool HasEra() const                         { return bHasEra; }
    void SetHasEra(sal_Bool bSet)                   { bHasEra = bSet; }

    void UpdateCalendar( const rtl::OUString& rNewCalendar );

    const LocaleDataWrapper& GetLocaleData() const;

    void AddToCode( const rtl::OUString& rString );
    void AddNumber( const SvXMLNumberInfo& rInfo );
    void AddCurrency( const rtl::OUString& rContent, LanguageType nLang );

    void AddNfKeyword( sal_uInt16 nIndex );
    sal_Bool ReplaceNfKeyword( sal_uInt16 nOld, sal_uInt16 nNew );
    void AddCondition( const sal_Int32 nIndex );
    void AddCondition( const sal_Int32 nIndex, const rtl::OUString& rFormat, const LocaleDataWrapper& rData );
    void AddCondition( const rtl::OUString& rCondition, const rtl::OUString& rApplyName );
    void AddColor( const Color& rColor );

    /// determine whether number format uses the system language
    sal_Bool IsSystemLanguage();
};

#endif

