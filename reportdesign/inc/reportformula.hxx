/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: reportformula.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-02 14:27:59 $
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

#ifndef REPORTFORMULA_HXX
#define REPORTFORMULA_HXX

#include "dllapi.h"

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
/** === end UNO includes === **/

#include <osl/diagnose.h>

//........................................................................
namespace rptui
{
//........................................................................

    //====================================================================
    //= ReportFormula
    //====================================================================
    class REPORTDESIGN_DLLPUBLIC ReportFormula
    {
    public:
        enum BindType
        {
            Expression,
            Field,

            Invalid
        };

    private:
        BindType            m_eType;
        ::rtl::OUString     m_sCompleteFormula;
        ::rtl::OUString     m_sUndecoratedContent;

    public:
        // constructs a ReportFormula instance which is initially invalid.
        ReportFormula();

        /// constructs a ReportFormula object from a string
        ReportFormula( const ::rtl::OUString& _rFormula );

        /// constructs a ReportFormula object from a string
        ReportFormula( const ::com::sun::star::uno::Any& _rWrappedFormula );

        /// constructs a ReportFormula by BindType
        ReportFormula( const BindType _eType, const ::rtl::OUString& _rFieldOrExpression );

        /// returns whether the object denotes a valid formula
        bool        isValid() const { return getType() != Invalid; }

        /// returns the type of the binding represented by the formula
        BindType    getType() const { return m_eType; }

        /// returns the complete formula represented by the object
        const ::rtl::OUString&
                    getCompleteFormula() const { return m_sCompleteFormula; }

        /** gets the <em>undecorated formula</em> content

            If the formula denotes a field binding, the <em>undecorated content</em> is the
            field name.

            If the formula denotes an expression, then the <em>undecorated content</em> is the expression
            itself.
        */
        const ::rtl::OUString& getUndecoratedContent() const { return m_sUndecoratedContent; }

        /// convenience alias for <code>getUndecoratedContent</code>, which asserts (in a non-product build) when used on an expression
        inline ::rtl::OUString  getFieldName() const;

        /// convenience alias for <code>getUndecoratedContent</code>, which asserts (in a non-product build) when used on a field
        inline ::rtl::OUString  getExpression() const;

        /** returns a bracketed field name of the formula denotes a field reference,
            or the undecorated expression if the formula denotes an expression.

            Effectively, this means the method returns the complete formular, stripped by the prefix
            which indicates a field or a expression.
        */
        ::rtl::OUString getBracketedFieldOrExpression() const;

    private:
        void    impl_construct( const ::rtl::OUString& _rFormula );
    };

    //--------------------------------------------------------------------
    inline ::rtl::OUString ReportFormula::getFieldName() const
    {
        OSL_PRECOND( getType() == Field, "ReportFormula::getFieldName: not bound to a field!" );
        return getUndecoratedContent();
    }

    //--------------------------------------------------------------------
    inline ::rtl::OUString ReportFormula::getExpression() const
    {
        OSL_PRECOND( getType() == Expression, "ReportFormula::getExpression: not bound to an expression!" );
        return getUndecoratedContent();
    }

//........................................................................
} // namespace rptui
//........................................................................

#endif // REPORTFORMULA_HXX
