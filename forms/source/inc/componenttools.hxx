/*************************************************************************
 *
 *  $RCSfile: componenttools.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-10-22 11:40:04 $
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

#ifndef FORMS_SOURCE_INC_COMPONENTTOOLS_HXX
#define FORMS_SOURCE_INC_COMPONENTTOOLS_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#include <com/sun/star/uno/Type.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
/** === end UNO includes === **/

#include <set>
#include <functional>

//........................................................................
namespace frm
{
//........................................................................

    struct TypeCompareLess : public ::std::binary_function< ::com::sun::star::uno::Type, ::com::sun::star::uno::Type, bool >
    {
    private:
        typedef ::com::sun::star::uno::Type             Type;

    public:
        bool operator()( const Type& _rLHS, const Type& _rRHS )
        {
            return _rLHS.getTypeName() < _rRHS.getTypeName();
        }
    };

    //====================================================================
    //= TypeBag
    //====================================================================
    /** a helper class which merges sequences of <type scope="com::sun::star::uno">Type</type>s,
        so that the resulting sequence contains every type at most once
    */
    class TypeBag
    {
    public:
        typedef ::com::sun::star::uno::Type             Type;
        typedef ::com::sun::star::uno::Sequence< Type > TypeSequence;
        typedef ::std::set< Type, TypeCompareLess >     TypeSet;

    private:
        TypeSet     m_aTypes;

    public:
        TypeBag(
            const TypeSequence& _rTypes1
        );

        TypeBag(
            const TypeSequence& _rTypes1,
            const TypeSequence& _rTypes2
        );
        TypeBag(
            const TypeSequence& _rTypes1,
            const TypeSequence& _rTypes2,
            const TypeSequence& _rTypes3
        );
        TypeBag(
            const TypeSequence& _rTypes1,
            const TypeSequence& _rTypes2,
            const TypeSequence& _rTypes3,
            const TypeSequence& _rTypes4
        );
        TypeBag(
            const TypeSequence& _rTypes1,
            const TypeSequence& _rTypes2,
            const TypeSequence& _rTypes3,
            const TypeSequence& _rTypes4,
            const TypeSequence& _rTypes5
        );

        void addTypes( const TypeSequence& _rTypes );

        /** returns the types represented by this bag
        */
        TypeSequence    getTypes() const;
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_INC_COMPONENTTOOLS_HXX

