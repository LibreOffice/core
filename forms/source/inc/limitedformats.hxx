/*************************************************************************
 *
 *  $RCSfile: limitedformats.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:01:23 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _FORMS_LIMITED_FORMATS_HXX_
#define _FORMS_LIMITED_FORMATS_HXX_

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#include <com/sun/star/beans/XFastPropertySet.hpp>

//.........................................................................
namespace frm
{
//.........................................................................

    //=====================================================================
    //= OLimitedFormats
    //=====================================================================
    /** maintains translation tables format key <-> enum value
        <p>Used for controls which provide a limited number for (standard) formats, which
        should be available as format keys.</p>
    */
    class OLimitedFormats
    {
    private:
        static sal_Int32    s_nInstanceCount;
        static ::osl::Mutex s_aMutex;
        static ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >
                            s_xStandardFormats;

    protected:
        sal_Int32           m_nFormatEnumPropertyHandle;
        const sal_Int16     m_nTableId;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet >
                            m_xAggregate;

    protected:
        /** ctor
            <p>The class id is used to determine the translation table to use. All instances which
            pass the same value here share one table.</p>
        */
        OLimitedFormats(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const sal_Int16 _nClassId
            );
        ~OLimitedFormats();

    protected:
        void setAggregateSet(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet >& _rxAggregate,
            sal_Int32 _nOriginalPropertyHandle
        );

    protected:
        void        getFormatKeyPropertyValue( ::com::sun::star::uno::Any& _rValue ) const;
        sal_Bool    convertFormatKeyPropertyValue(
                        ::com::sun::star::uno::Any& _rConvertedValue,
                        ::com::sun::star::uno::Any& _rOldValue,
                const   ::com::sun::star::uno::Any& _rNewValue
            );
        void        setFormatKeyPropertyValue( const ::com::sun::star::uno::Any& _rNewValue );
        // setFormatKeyPropertyValue should only be called with a value got from convertFormatKeyPropertyValue!

        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >
                    getFormatsSupplier() const { return s_xStandardFormats; }

    private:
        void acquireSupplier(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
        void releaseSupplier();

        static void ensureTableInitialized(const sal_Int16 _nTableId);
        static void clearTable(const sal_Int16 _nTableId);
    };

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FORMS_LIMITED_FORMATS_HXX_

