/*************************************************************************
 *
 *  $RCSfile: TableRow.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:36 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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

