/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formattedcolumnvalue.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 14:48:19 $
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

#ifndef CONNECTIVITY_FORMATTEDCOLUMNVALUE_HXX
#define CONNECTIVITY_FORMATTEDCOLUMNVALUE_HXX

/** === begin UNO includes === **/
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdb/XColumnUpdate.hpp>
/** === end UNO includes === **/

#include <boost/noncopyable.hpp>

#include <memory>

namespace comphelper { class ComponentContext; }

//........................................................................
namespace dbtools
{
//........................................................................

    struct FormattedColumnValue_Data;
    //====================================================================
    //= FormattedColumnValue
    //====================================================================
    /** a class which helps retrieving and setting the value of a database column
        as formatted string.
    */
    class FormattedColumnValue : public ::boost::noncopyable
    {
    public:
        /** constructs an instance

            The format key for the string value exchange is taken from the given column object.
            If it has a non-<NULL/> property value <code>FormatKey</code>, this key is taken.
            Otherwise, a default format matching the column type is determined.

            The locale of this fallback format is the current system locale.

            The number formats supplier is determined from the given <code>RowSet</code>, by
            examining its <code>ActiveConnection</code>.
        */
        FormattedColumnValue(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rxRowSet,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn
        );

        // note that all methods of this class need to be virtual, since it's
        // used in a load-on-demand context in module SVX

        virtual ~FormattedColumnValue();

        virtual void clear();

        // access to the details of the formatting we determined
        virtual sal_Int32   getFormatKey() const;
        virtual sal_Int32   getFieldType() const;
        virtual sal_Int16   getKeyType() const;
        virtual bool        isNumericField() const;
        virtual const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >&
                            getColumn() const;
        virtual const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumnUpdate >&
                            getColumnUpdate() const;

        virtual bool            setFormattedValue( const ::rtl::OUString& _rFormattedStringValue ) const;
        virtual ::rtl::OUString getFormattedValue() const;

    private:
        ::std::auto_ptr< FormattedColumnValue_Data >    m_pData;
    };

//........................................................................
} // namespace dbtools
//........................................................................

#endif // CONNECTIVITY_FORMATTEDCOLUMNVALUE_HXX
