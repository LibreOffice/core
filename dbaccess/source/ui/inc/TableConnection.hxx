/*************************************************************************
 *
 *  $RCSfile: TableConnection.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:21:13 $
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
#ifndef DBAUI_TABLECONNECTION_HXX
#define DBAUI_TABLECONNECTION_HXX

//#ifndef _SV_GEN_HXX
//#include <tools/gen.hxx>
//#endif
//#ifndef DBAUI_CONNECTIONLINE_HXX
//#include "ConnectionLine.hxx"
//#endif
#include <vector>
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

namespace com { namespace sun { namespace star { namespace accessibility
{
    class XAccessible;
}}}}

class Point;
class Rectangle;

namespace dbaui
{
    class OTableConnectionData;
    class OTableWindow;
    class OJoinTableView;
    class OConnectionLine;

    DBG_NAMEEX(OTableConnection)
    class OTableConnection
    {
        ::std::vector<OConnectionLine*> m_vConnLine;
        OTableConnectionData*           m_pData;
        OJoinTableView*                 m_pParent;
        mutable ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > m_xAccessible;

        BOOL                            m_bSelected;

        void Init();
        /** clearLineData loops through the vector and deletes all lines
        */
        void clearLineData();

    protected:
        OConnectionLine* CreateConnLine( const OConnectionLine& rConnLine );

    public:
        TYPEINFO();
        OTableConnection( OJoinTableView* pContainer, OTableConnectionData* pTabConnData );
        OTableConnection( const OTableConnection& rConn );
            // WICHTIG : normalerweise bekomme ich von aussen einen Zeiger auf OTableConnectionData mitgegeben, hier aber muss ich
            // mir (ueber OTableConnectionData::NewInstance) selber eine INstanz anlegen, die ich aber - wie in allen anderen Faellen auch -
            // NIE loesche. Der Aufrufer ist also bei Benutzung dieses Constructors dafuer verantwortlich, meine Daten abzufragen und
            // sich irgendwo zu merken, um sie dann irgendwann zu loeschen.
        virtual ~OTableConnection();

        OTableConnection& operator=( const OTableConnection& rConn );


        void        Select();
        void        Deselect();
        BOOL        IsSelected() const { return m_bSelected; }
        BOOL        CheckHit( const Point& rMousePos ) const;
        bool        Invalidate();
        void        UpdateLineList();

        OTableWindow* GetSourceWin() const;
        OTableWindow* GetDestWin() const;

        bool RecalcLines();
        /** isTableConnection
            @param  _pTable the table where we should check if we belongs to it

            @return true when the source or the destination window are equal
        */
        bool isTableConnection(const OTableWindow* _pTable)
        {
            return (_pTable == GetSourceWin() || _pTable == GetDestWin());
        }

        Rectangle   GetBoundingRect() const;

        OTableConnectionData*           GetData() const { return m_pData; }
        const ::std::vector<OConnectionLine*>* GetConnLineList() const { return &m_vConnLine; }
        OJoinTableView*                 GetParent() const { return m_pParent; }
        virtual void                    Draw( const Rectangle& rRect );
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > getAccessible() const;
    };
}
#endif // DBAUI_TABLECONNECTION_HXX

