/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#include "TableConnectionData.hxx"

class Point;
class Rectangle;

namespace dbaui
{
    class OTableConnectionData;
    class OTableWindow;
    class OJoinTableView;
    class OConnectionLine;

    DBG_NAMEEX(OTableConnection)
    class OTableConnection : public Window
    {
        ::std::vector<OConnectionLine*> m_vConnLine;
        TTableConnectionData::value_type
                                        m_pData;
        OJoinTableView*                 m_pParent;

        sal_Bool                            m_bSelected;

        void Init();
        /** clearLineData loops through the vector and deletes all lines
        */
        void clearLineData();

    protected:
        OConnectionLine* CreateConnLine( const OConnectionLine& rConnLine );

    public:
        OTableConnection( OJoinTableView* pContainer, const TTableConnectionData::value_type& pTabConnData );
        OTableConnection( const OTableConnection& rConn );
            // WICHTIG : normalerweise bekomme ich von aussen einen Zeiger auf OTableConnectionData mitgegeben, hier aber muss ich
            // mir (ueber OTableConnectionData::NewInstance) selber eine INstanz anlegen, die ich aber - wie in allen anderen Faellen auch -
            // NIE loesche. Der Aufrufer ist also bei Benutzung dieses Constructors dafuer verantwortlich, meine Daten abzufragen und
            // sich irgendwo zu merken, um sie dann irgendwann zu loeschen.
        virtual ~OTableConnection();

        OTableConnection& operator=( const OTableConnection& rConn );


        void        Select();
        void        Deselect();
        sal_Bool        IsSelected() const { return m_bSelected; }
        sal_Bool        CheckHit( const Point& rMousePos ) const;
        bool        InvalidateConnection();
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

        inline TTableConnectionData::value_type GetData() const { return m_pData; }
        const ::std::vector<OConnectionLine*>*  GetConnLineList() const { return &m_vConnLine; }
        inline OJoinTableView*                  GetParent() const { return m_pParent; }
        virtual void                    Draw( const Rectangle& rRect );
        using Window::Draw;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();
    };
}
#endif // DBAUI_TABLECONNECTION_HXX

