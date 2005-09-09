/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: numberformatcodewrapper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:33:27 $
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

#ifndef _UNOTOOLS_NUMBERFORMATCODEWRAPPER_HXX
#define _UNOTOOLS_NUMBERFORMATCODEWRAPPER_HXX

#ifndef _COM_SUN_STAR_I18N_XNUMBERFORMATCODE_HPP_
#include <com/sun/star/i18n/XNumberFormatCode.hpp>
#endif
#ifndef INCLUDED_UNOTOOLSDLLAPI_H
#include "unotools/unotoolsdllapi.h"
#endif

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
}}}


class UNOTOOLS_DLLPUBLIC NumberFormatCodeWrapper
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XNumberFormatCode >   xNFC;
    ::com::sun::star::lang::Locale          aLocale;

                                // not implemented, prevent usage
                                NumberFormatCodeWrapper( const NumberFormatCodeWrapper& );
            NumberFormatCodeWrapper&    operator=( const NumberFormatCodeWrapper& );

public:
                                NumberFormatCodeWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF,
                                    const ::com::sun::star::lang::Locale& rLocale
                                    );

                                ~NumberFormatCodeWrapper();


    /// set a new Locale
            void                setLocale( const ::com::sun::star::lang::Locale& rLocale );

    /// get current Locale
    const ::com::sun::star::lang::Locale& getLocale() const { return aLocale; }


    // Wrapper implementations of class NumberFormatCodeMapper

    ::com::sun::star::i18n::NumberFormatCode getDefault( sal_Int16 nFormatType, sal_Int16 nFormatUsage ) const;
    ::com::sun::star::i18n::NumberFormatCode getFormatCode( sal_Int16 nFormatIndex ) const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > getAllFormatCode( sal_Int16 nFormatUsage ) const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > getAllFormatCodes() const;

};

#endif // _UNOTOOLS_NUMBERFORMATCODEWRAPPER_HXX
