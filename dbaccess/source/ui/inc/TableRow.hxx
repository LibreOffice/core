/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableRow.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-01-16 15:28:26 $
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
#ifndef DBAUI_TABLEROW_HXX
#define DBAUI_TABLEROW_HXX

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif


namespace dbaui
{
//  class OTableRow;
//  friend SvStream& operator<<( SvStream& rStr, OTableRow& _rRow );

    class OFieldDescription;
    class OTypeInfo;
    class OTableRow
    {
    private:
        OFieldDescription*      m_pActFieldDescr;
        long                    m_nPos;
        bool                    m_bReadOnly;
        bool                    m_bOwnsDescriptions;

    protected:
    public:
        OTableRow();
        OTableRow(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xAffectedCol);
        OTableRow( const OTableRow& rRow, long nPosition = -1 );
        ~OTableRow();

        inline OFieldDescription* GetActFieldDescr() const { return m_pActFieldDescr; }
        inline bool isValid() const { return GetActFieldDescr() != NULL; }

        void SetFieldType( const TOTypeInfoSP& _pType, sal_Bool _bForce = sal_False );

        void SetPrimaryKey( BOOL bSet );
        BOOL IsPrimaryKey() const;

        /** returns the current position in the table.
            @return
                the current position in the table
        */
        inline long GetPos() const { return m_nPos; }
        inline void SetPos(sal_Int32 _nPos) { m_nPos = _nPos; }

        /** set the row readonly
            @param  _bRead
                if <TRUE/> then the row is redonly, otherwise not
        */
        inline void SetReadOnly( bool _bRead=true ){ m_bReadOnly = _bRead; }

        /** returns if the row is readonly
            @return
                <TRUE/> if readonly, otherwise <FALSE/>
        */
        inline bool IsReadOnly() const { return m_bReadOnly; }

        friend SvStream& operator<<( SvStream& rStr,const OTableRow& _rRow );
        friend SvStream& operator>>( SvStream& rStr, OTableRow& _rRow );
    };
}
#endif // DBAUI_TABLEROW_HXX

