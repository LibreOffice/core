/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableWindowData.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:38:38 $
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

