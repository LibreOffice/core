/*************************************************************************
 *
 *  $RCSfile: filter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-01-28 14:34:02 $
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

#ifndef __FRAMEWORK_CONSTANT_FILTER_H_
#define __FRAMEWORK_CONSTANT_FILTER_H_

#include <rtl/ustring.hxx>

namespace framework{
    namespace constant{

struct Filter
{
    public:

        static const ::rtl::OUString PROP_NAME;                 // string
        static const ::rtl::OUString PROP_TYPE;                 // string
        static const ::rtl::OUString PROP_DOCUMENTSERVICE;      // string
        static const ::rtl::OUString PROP_FILTERSERVICE;        // string
        static const ::rtl::OUString PROP_UICOMPONENT;          // string
        static const ::rtl::OUString PROP_FLAGS;                // int32
        static const ::rtl::OUString PROP_USERDATA;             // seq< string >
        static const ::rtl::OUString PROP_TEMPLATENAME;         // string

        static const ::rtl::OUString QUERY_GET_DEFAULT_FILTER_FOR_TYPE;
        static const ::rtl::OUString QUERY_ALL;
        static const ::rtl::OUString QUERY_WRITER;
        static const ::rtl::OUString QUERY_WEB;
        static const ::rtl::OUString QUERY_GLOBAL;
        static const ::rtl::OUString QUERY_CHART;
        static const ::rtl::OUString QUERY_CALC;
        static const ::rtl::OUString QUERY_IMPRESS;
        static const ::rtl::OUString QUERY_DRAW;
        static const ::rtl::OUString QUERY_MATH;

        static const ::rtl::OUString QUERYPARAM_IFLAGS;
        static const ::rtl::OUString QUERYPARAM_EFLAGS;
        static const ::rtl::OUString QUERYPARAM_SORT_PROP;
        static const ::rtl::OUString QUERYPARAM_DESCENDING;
        static const ::rtl::OUString QUERYPARAM_USE_ORDER;
        static const ::rtl::OUString QUERYPARAM_DEFAULT_FIRST;
        static const ::rtl::OUString QUERYPARAM_CASE_SENSITIVE;
        static const ::rtl::OUString QUERYPARAMVALUE_SORT_PROP_NAME;
        static const ::rtl::OUString QUERYPARAMVALUE_SORT_PROP_UINAME;

        static const ::rtl::OUString FLAGNAME_IMPORT;
        static const ::rtl::OUString FLAGNAME_EXPORT;
        static const ::rtl::OUString FLAGNAME_TEMPLATE;
        static const ::rtl::OUString FLAGNAME_INTERNAL;
        static const ::rtl::OUString FLAGNAME_TEMPLATEPATH;
        static const ::rtl::OUString FLAGNAME_OWN;
        static const ::rtl::OUString FLAGNAME_ALIEN;
        static const ::rtl::OUString FLAGNAME_USESOPTIONS;
        static const ::rtl::OUString FLAGNAME_DEFAULT;
        static const ::rtl::OUString FLAGNAME_EXECUTABLE;
        static const ::rtl::OUString FLAGNAME_SUPPORTSSELECTION;
        static const ::rtl::OUString FLAGNAME_MAPTOAPPPLUG;
        static const ::rtl::OUString FLAGNAME_NOTINFILEDIALOG;
        static const ::rtl::OUString FLAGNAME_NOTINCHOOSER;
        static const ::rtl::OUString FLAGNAME_ASYNCHRON;
        static const ::rtl::OUString FLAGNAME_CREATOR;
        static const ::rtl::OUString FLAGNAME_READONLY;
        static const ::rtl::OUString FLAGNAME_NOTINSTALLED;
        static const ::rtl::OUString FLAGNAME_CONSULTSERVICE;
        static const ::rtl::OUString FLAGNAME_3RDPARTYFILTER;
        static const ::rtl::OUString FLAGNAME_PACKED;
        static const ::rtl::OUString FLAGNAME_SILENTEXPORT;
        static const ::rtl::OUString FLAGNAME_BROWSERPREFERED;
        static const ::rtl::OUString FLAGNAME_PREFERED;

        static const sal_Int32       FLAGVALUE_IMPORT;
        static const sal_Int32       FLAGVALUE_EXPORT;
        static const sal_Int32       FLAGVALUE_TEMPLATE;
        static const sal_Int32       FLAGVALUE_INTERNAL;
        static const sal_Int32       FLAGVALUE_TEMPLATEPATH;
        static const sal_Int32       FLAGVALUE_OWN;
        static const sal_Int32       FLAGVALUE_ALIEN;
        static const sal_Int32       FLAGVALUE_USESOPTIONS;
        static const sal_Int32       FLAGVALUE_DEFAULT;
        static const sal_Int32       FLAGVALUE_EXECUTABLE;
        static const sal_Int32       FLAGVALUE_SUPPORTSSELECTION;
        static const sal_Int32       FLAGVALUE_MAPTOAPPPLUG;
        static const sal_Int32       FLAGVALUE_NOTINFILEDIALOG;
        static const sal_Int32       FLAGVALUE_NOTINCHOOSER;
        static const sal_Int32       FLAGVALUE_ASYNCHRON;
        static const sal_Int32       FLAGVALUE_CREATOR;
        static const sal_Int32       FLAGVALUE_READONLY;
        static const sal_Int32       FLAGVALUE_NOTINSTALLED;
        static const sal_Int32       FLAGVALUE_CONSULTSERVICE;
        static const sal_Int32       FLAGVALUE_3RDPARTYFILTER;
        static const sal_Int32       FLAGVALUE_PACKED;
        static const sal_Int32       FLAGVALUE_SILENTEXPORT;
        static const sal_Int32       FLAGVALUE_BROWSERPREFERED;
        //FREE! ... 0x00800000L
        static const sal_Int32       FLAGVALUE_PREFERED;

        //_______________________________________________

        /** @short  checks if the given flag mask is set.

            @param  nField
                    the flag field, which should be checked.

            @param  nMask
                    these flags should be checked against the field.

            @return <TRUE/> if mask match to the given flag field.
         */
        sal_Bool areFlagsSet(sal_Int32 nField, sal_Int32 nMask);

        //_______________________________________________

        /** @short  set the specified flags on the given field.

            @param  nField
                    the flag field, which should be changed.

            @param  nMask
                    this flags field should be set on the field.
         */
        void addFlags(sal_Int32& nField, sal_Int32 nFlags);

        //_______________________________________________

        /** @short  remove the specified flags from the given field.

            @param  nField
                    the flag field, which should be changed.

            @param  nMask
                    this flags field should be removed from the field.
         */
        void removeFlags(sal_Int32& nField, sal_Int32 nFlags);
};

    } // namespace constant
} // namespace framework

#endif // #ifndef __FRAMEWORK_CONSTANT_FILTER_H_
