/*************************************************************************
 *
 *  $RCSfile: roadmap.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2004-11-26 20:39:57 $
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
#ifndef _SVTOOLS_ROADMAP_HXX
#define _SVTOOLS_ROADMAP_HXX

#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif

#ifndef _SV_IMGCTRL_HXX
#include <vcl/imgctrl.hxx>
#endif

#ifndef _SVTOOLS_HYPERLABEL_HXX
#include "hyperlabel.hxx"
#endif




class Bitmap;
//.........................................................................
namespace svt
{
//.........................................................................

    struct RoadmapTypes
    {
    public:
        typedef sal_Int16 ItemId;
        typedef sal_Int32 ItemIndex;
    };


    class RoadmapImpl;


    class ORoadmapIDHyperLabel :  public FixedText
    {
    public:
        ORoadmapIDHyperLabel( Window* _pParent, const ResId& _rId );
        ORoadmapIDHyperLabel( Window* _pParent, WinBits _nWinStyle = 0 );
        ~ORoadmapIDHyperLabel( );
        virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    };

    class ORoadmapHyperLabel : public RoadmapTypes, public OutputDevice
    {
    private:
        ORoadmapIDHyperLabel* mpIDLabel;
        HyperLabel* mpDescHyperLabel;


    public:

        ORoadmapHyperLabel( Window* _pParent, const ResId& _rId );
        ORoadmapHyperLabel( Window* _pParent, WinBits _nWinStyle = 0 );
        ~ORoadmapHyperLabel( );

        void                    SetID( sal_Int16 _ID );
        sal_Int16               GetID() const;

        void                    SetIndex( sal_Int32 _Index );
        sal_Int32               GetIndex() const;

        void                    SetLabel( ::rtl::OUString _rText );
        ::rtl::OUString         GetLabel( );

        void                    SetLabelAndSize( ItemIndex _RMIndex, ::rtl::OUString _rText, const Size& rNewSize);

        void                    SetPosition( ORoadmapHyperLabel* OldHyperLabel );
        Point                   GetLogicalPosition();

        void                    ToggleBackgroundColor( const Color& _rGBColor );
        void                    SetInteractive( sal_Bool _bInteractive );

        void                    SetClickHdl( const Link& rLink );
        const Link&             GetClickHdl() const;
        void                    SetZOrder( ORoadmapHyperLabel* pRefRoadmapHyperLabel, USHORT nFlags );
        void                    Enable( BOOL bEnable = TRUE);
        BOOL                    IsEnabled() const;
        void                    GrabFocus();

        void                    SetIDLabel(ORoadmapIDHyperLabel* _pIDLabel){mpIDLabel = _pIDLabel;};
        ORoadmapIDHyperLabel*   GetIDLabel() const { return mpIDLabel;};

        HyperLabel*             GetDescriptionHyperLabel() const { return mpDescHyperLabel;};
        void                    SetDescriptionHyperLabel(HyperLabel* _pDescriptionHyperLabel){mpDescHyperLabel = _pDescriptionHyperLabel;};

    };



    //=====================================================================
    //= Roadmap
    //=====================================================================
    class ORoadmap : public Control, public RoadmapTypes
    {
    protected:
        RoadmapImpl*    m_pImpl;
        // Window overridables
        void            Paint( const Rectangle& _rRect );
        void            implInit();

    public:
        ORoadmap( Window* _pParent, const ResId& _rId );
        ORoadmap( Window* _pParent, WinBits _nWinStyle = 0 );
        ~ORoadmap( );

        void            SetRoadmapBitmap( const BitmapEx& maBitmap, sal_Bool _bInvalidate = sal_True );
        const BitmapEx& GetRoadmapBitmap( ) const;

        void            EnableRoadmapItem( ItemId _nItemId, sal_Bool _bEnable, ItemIndex _nStartIndex = 0  );
        sal_Bool        IsRoadmapItemEnabled( ItemId _nItemId, ItemIndex _nStartIndex = 0 ) const;

        void            ChangeRoadmapItemLabel( ItemId _nID, ::rtl::OUString sLabel, ItemIndex _nStartIndex = 0 );
        void            ChangeRoadmapItemID( ItemId _nID, ItemId _NewID, ItemIndex _nStartIndex = 0  );

        void            SetRoadmapInteractive( sal_Bool _bInteractive );
        sal_Bool        IsRoadmapInteractive();

        void            SetRoadmapComplete( sal_Bool _bComplete );
        sal_Bool        IsRoadmapComplete() const;

        ItemIndex       GetItemCount() const;
        ItemId          GetItemID( ItemIndex _nIndex ) const;

        void            InsertRoadmapItem( ItemIndex _Index, ::rtl::OUString _RoadmapItem, ItemId _nUniqueId, sal_Bool _bEnabled = sal_True );
        void            ReplaceRoadmapItem( ItemIndex _Index, ::rtl::OUString _RoadmapItem, ItemId _nUniqueId, sal_Bool _bEnabled );
        void            DeleteRoadmapItem( ItemIndex _nIndex );

        ItemId          GetCurrentRoadmapItemID() const;
        sal_Bool        SelectRoadmapItemByID( ItemId _nItemID );

        void            SetItemSelectHdl( const Link& _rHdl );
        Link            GetItemSelectHdl( ) const;
        virtual void    DataChanged( const DataChangedEvent& rDCEvt );


    protected:
        sal_Int32       PreNotify( NotifyEvent& rNEvt );

    protected:
        /// called when an item has been selected by any means
        virtual void    Select();

    private:
        DECL_LINK(ImplClickHdl, HyperLabel*);

        ORoadmapHyperLabel*         GetByIndex( ItemIndex _nItemIndex );
        const ORoadmapHyperLabel*   GetByIndex( ItemIndex _nItemIndex ) const;

        ORoadmapHyperLabel*         GetByID( ItemId _nID, ItemIndex _nStartIndex = 0  );
        const ORoadmapHyperLabel*   GetByID( ItemId _nID, ItemIndex _nStartIndex = 0  ) const;
        ORoadmapHyperLabel*         GetPreviousHyperLabel( ItemIndex _Index);

        void                        DrawHeadline();
        void                        DeselectOldRoadmapItems();
        ItemId                      GetNextAvailableItemId( ItemIndex _NewIndex );
        ItemId                      GetPreviousAvailableItemId( ItemIndex _NewIndex );
        ORoadmapHyperLabel*         GetByPointer(Window* pWindow);
        void                        InitializeHyperLabelSize();
        ORoadmapHyperLabel*         InsertHyperLabel( ItemIndex _Index, ::rtl::OUString _aStr, ItemId _RMID, sal_Bool _bEnabled = sal_True  );
        void                        UpdatefollowingHyperLabels( ItemIndex _Index, sal_Int16 _nadd = 1);
        void                        SetRoadmapLabel(ORoadmapHyperLabel* CurHyperLabel, sal_Int32 _nPrefix, String _sDescription);
    };

//.........................................................................
}   // namespace svt
//.........................................................................

#endif

