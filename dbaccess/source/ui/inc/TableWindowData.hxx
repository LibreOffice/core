/*************************************************************************
 *
 *  $RCSfile: TableWindowData.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:05:14 $
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
#ifndef DBAUI_TABLEWINDOWDATA_HXX
#define DBAUI_TABLEWINDOWDATA_HXX

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif

namespace dbaui
{
    class OTableWindowData
    {
    protected:
        ::rtl::OUString m_aTableName;
        ::rtl::OUString m_aWinName;
        ::rtl::OUString m_sComposedName;
        Point           m_aPosition;
        Size            m_aSize;
        sal_Bool        m_bShowAll;

    public:
        TYPEINFO();
        OTableWindowData();
        OTableWindowData( const ::rtl::OUString& _rComposedName, const ::rtl::OUString& strTableName, const ::rtl::OUString& rWinName = ::rtl::OUString() );
        virtual ~OTableWindowData();

        virtual void Load(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxIn);
        virtual void Save(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOut);

        ::rtl::OUString GetComposedName()   const { return m_sComposedName; }
        ::rtl::OUString GetTableName()      const { return m_aTableName; }
        ::rtl::OUString GetWinName()        const { return m_aWinName; }
        Point GetPosition()                 const { return m_aPosition; }
        Size GetSize()                      const { return m_aSize; }
        BOOL IsShowAll()                    const { return m_bShowAll; }
        BOOL HasPosition()  const;
        BOOL HasSize()      const;

        void SetTableName( const ::rtl::OUString& rTableName )  { m_aTableName = rTableName; }
        void SetWinName( const ::rtl::OUString& rWinName )      { m_aWinName = rWinName; }
        void SetPosition( const Point& rPos )                   { m_aPosition=rPos; }
        void SetSize( const Size& rSize )                       { m_aSize = rSize; }
        void ShowAll( BOOL bAll )                               { m_bShowAll = bAll; }
    };
}
#endif // DBAUI_TABLEWINDOWDATA_HXX

