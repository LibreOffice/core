/*************************************************************************
#*
#*  $RCSfile: numberformatcode.hxx,v $
#*
#*  NumberFormatCodeMapper
#*
#*  Creation:   SSE 08/12/2000
#*
#*  Last change:    $Author: bustamam $ $Date: 2001-08-30 23:33:16 $
#*
#*  $Revision: 1.1 $
#*
#*  $Source: /zpool/svn/migration/cvs_rep_09_09_08/code/i18npool/inc/numberformatcode.hxx,v $
#*
#*  Copyright (c) 2000 Sun Microsystems Inc.
#*
#*************************************************************************/

#ifndef _I18N_NUMBERFORMATCODE_HXX_
#define _I18N_NUMBERFORMATCODE_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase1.hxx> // helper for implementations

#include <com/sun/star/i18n/XNumberFormatCode.hpp>
#include <com/sun/star/i18n/XLocaleData.hpp>

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif


class NumberFormatCodeMapper : public cppu::WeakImplHelper1
<
    ::com::sun::star::i18n::XNumberFormatCode
>
{
public:
    NumberFormatCodeMapper( const ::com::sun::star::uno::Reference <
                    ::com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    ~NumberFormatCodeMapper();

    virtual ::com::sun::star::i18n::NumberFormatCode SAL_CALL getDefault( sal_Int16 nFormatType, sal_Int16 nFormatUsage, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::i18n::NumberFormatCode SAL_CALL getFormatCode( sal_Int16 nFormatIndex, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > SAL_CALL getAllFormatCode( sal_Int16 nFormatUsage, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > SAL_CALL getAllFormatCodes( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::lang::Locale aLocale;
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > xMSF;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::FormatElement > aFormatSeq;
    ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XLocaleData > xlocaleData;
    sal_Bool bFormatsValid;

    void setupLocale( const ::com::sun::star::lang::Locale& rLocale );
    void getFormats( const ::com::sun::star::lang::Locale& rLocale );
    ::rtl::OUString mapElementTypeShortToString(sal_Int16 formatType);
    sal_Int16 mapElementTypeStringToShort(const ::rtl::OUString& formatType);
    ::rtl::OUString mapElementUsageShortToString(sal_Int16 formatUsage);
    sal_Int16 mapElementUsageStringToShort(const ::rtl::OUString& formatUsage);
    void createLocaleDataObject();
};


#endif // _I18N_NUMBERFORMATCODE_HXX_
