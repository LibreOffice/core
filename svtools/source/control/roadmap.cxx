/*************************************************************************
 *
 *  $RCSfile: roadmap.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-05-19 14:01:05 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
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
#include <roadmap.hxx>
#endif

#ifndef _STRING_HXX
#define _STRING_HXX
#endif

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

#include <algorithm>

#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/OUString.hxx>
#endif

#define RMENTRYPOINT_X      4
#define RMENTRYPOINT_Y      27
#define RMITEMDISTANCE_Y    18
#define RMINCOMPLETE        -1
#define NREMOVERMITEM       -1
#define NADDITEM            1
#define INCOMPLETELABEL     ::String::CreateFromAscii("...")        // TODO: Cast to String

//.........................................................................
namespace svt
{
//.........................................................................

    typedef std::vector< ::rtl::OUString > S_Vector;
    typedef std::vector< HyperLabel* > HL_Vector;

    //=====================================================================
    //= FontChanger
    //=====================================================================
    class FontChanger
    {
    protected:
        OutputDevice*   m_pDev;

    public:
        FontChanger( OutputDevice* _pDev, const Font& _rNewFont )
            :m_pDev( _pDev )
        {
            m_pDev->Push( PUSH_FONT );
            m_pDev->SetFont( _rNewFont );
        }

        ~FontChanger()
        {
            m_pDev->Pop( );
        }
    };


    //=====================================================================
    //= ColorChanger
    //=====================================================================
    class ColorChanger
    {
    protected:
        OutputDevice*   m_pDev;

    public:
        ColorChanger( OutputDevice* _pDev, const Color& _rNewLineColor, const Color& _rNewFillColor )
            :m_pDev( _pDev )
        {
            m_pDev->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
            m_pDev->SetLineColor( _rNewLineColor );
            m_pDev->SetFillColor( _rNewFillColor );
        }

        ~ColorChanger()
        {
            m_pDev->Pop();
        }
    };


    //=====================================================================
    //= RoadmapImpl
    //=====================================================================
    class RoadmapImpl : public RoadmapTypes
    {
    protected:
        Link                m_aSelectHdl;
        BitmapEx            m_aPicture;
        HL_Vector           m_aRoadmapSteps;
        ItemId              m_iCurItemID;
        sal_Bool            m_bInteractive;
        sal_Bool            m_bComplete;

    public:
        Size aHyperLabelPixelSize;
        HyperLabel* InCompleteHyperLabel;

        void                addHyperLabel( HyperLabel*  _rRoadmapStep ) { m_aRoadmapSteps.push_back(_rRoadmapStep); }

        HL_Vector&          getHyperLabels() { return m_aRoadmapSteps; }
        const HL_Vector&    getHyperLabels() const { return m_aRoadmapSteps; }

        void                insertHyperLabel( ItemIndex _Index, HyperLabel* _rRoadmapStep ) { m_aRoadmapSteps.insert( m_aRoadmapSteps.begin() + _Index, _rRoadmapStep ); }

        ItemIndex           getItemCount() const { return m_aRoadmapSteps.size();}

        void                setCurItemID( ItemId i ) {m_iCurItemID = i; }
        ItemId              getCurItemID() const { return m_iCurItemID; }

        void                setInteractive(const sal_Bool _bInteractive) {m_bInteractive = _bInteractive; }
        sal_Bool            isInteractive() const { return m_bInteractive; };

        void                setComplete(const sal_Bool _bComplete) {m_bComplete = _bComplete; }
        sal_Bool            isComplete() const { return m_bComplete; };

        void                setPicture( const BitmapEx& _rPic ) { m_aPicture = _rPic; }
        const BitmapEx&     getPicture( ) const { return m_aPicture; }

        void                setSelectHdl( const Link& _rHdl ) { m_aSelectHdl = _rHdl; }
        const Link&         getSelectHdl( ) const { return m_aSelectHdl; }

        void removeHyperLabel( ItemIndex _Index )
        {
            if ( ( _Index > -1 ) && ( _Index < getItemCount() ) )
            {
                delete m_aRoadmapSteps[_Index];
                m_aRoadmapSteps.erase( m_aRoadmapSteps.begin() + _Index);
            }
        }
    };

    //---------------------------------------------------------------------


    //=====================================================================
    //= Roadmap
    //=====================================================================
    //---------------------------------------------------------------------
    ORoadmap::ORoadmap( Window* _pParent, const ResId& _rId ):Control( _pParent, _rId )
                                                            ,m_pImpl( new RoadmapImpl() )
    {
        implInit();
    }

    //---------------------------------------------------------------------
    ORoadmap::ORoadmap( Window* _pParent, WinBits _nWinStyle )
        :Control( _pParent, _nWinStyle )
        , m_pImpl( new RoadmapImpl() )

    {
        implInit();
    }

    //---------------------------------------------------------------------
    void ORoadmap::implInit()
    {
        Font aFont = GetFont( );
        aFont.SetWeight( WEIGHT_BOLD );
        aFont.SetUnderline( UNDERLINE_SINGLE );
        SetFont( aFont );

        m_pImpl->InCompleteHyperLabel = NULL;
        m_pImpl->setCurItemID(-1 );
        m_pImpl->setComplete( sal_True );
// TODO: if somebody sets a new font from outside (OutputDevice::SetFont), we would have to react
// on this with calculating a new bold font.
// Unfortunately, the OutputDevice does not offer a notify mechanism for a changed font.
// So settings the font from outside is simply a forbidded scenario at the moment
    }

    //---------------------------------------------------------------------
    ORoadmap::~ORoadmap( )
    {
        HL_Vector pLocRoadmapItems = m_pImpl->getHyperLabels();
        for ( HL_Vector::iterator i = pLocRoadmapItems.begin(); i< pLocRoadmapItems.end(); i++)
        {
            delete *i;
        }
        if ( ! m_pImpl->isComplete() )
            delete m_pImpl->InCompleteHyperLabel;
        delete m_pImpl;
    }


    RoadmapTypes::ItemId ORoadmap::GetCurrentRoadmapItemID() const
    {
        return m_pImpl->getCurItemID();
    }


    void ORoadmap::InitializeHyperLabelSize()
    {
        Size aSize = GetSizePixel();
        Size aLogicSize = PixelToLogic( aSize, MAP_APPFONT );
        aLogicSize.Height() = LABELBASEMAPHEIGHT;
        aLogicSize.Width() -= 10;
        m_pImpl->aHyperLabelPixelSize = LogicToPixel( aLogicSize, MAP_APPFONT );
        EnableMapMode( sal_False );
    }


    ::rtl::OUString ORoadmap::ConcatenateRMItemLabel( ItemIndex _RMIndex, ::rtl::OUString _sLabel )
    {
        ::rtl::OUString aStr = ::rtl::OUString::createFromAscii("");
        aStr = ::rtl::OUString::valueOf( (sal_Int32)( _RMIndex + 1 ) ) +  ::rtl::OUString::createFromAscii( ". " );
        aStr += _sLabel;
        return aStr;
    }


       //---------------------------------------------------------------------

    HyperLabel* ORoadmap::InsertHyperLabel( ItemIndex _Index, ::rtl::OUString _sLabel, ItemId _RMID, sal_Bool _bEnabled)
    {
        sal_Int32 _YPos = RMENTRYPOINT_Y + ( _Index * RMITEMDISTANCE_Y);
        if (m_pImpl->getItemCount() == 0 )
            InitializeHyperLabelSize();
        HyperLabel* CurHyperLabel;
        HyperLabel* OldHyperLabel;
        ::rtl::OUString sLabel = ConcatenateRMItemLabel( _Index, _sLabel );
        if (_RMID != RMINCOMPLETE )
        {
            if (_Index == 0){        // Todo: Wie mache ich die Controls über Key up and down accessible
                CurHyperLabel = new HyperLabel(this, WB_TABSTOP | WB_WORDBREAK);     //WB_GROUP |
                CurHyperLabel->SetZOrder( NULL, WINDOW_ZORDER_FIRST );
            }
            else
            {
                CurHyperLabel = new HyperLabel(this, WB_TABSTOP | WB_WORDBREAK);
                OldHyperLabel = m_pImpl->getHyperLabels().at( _Index - 1 );
                CurHyperLabel->SetZOrder( OldHyperLabel, WINDOW_ZORDER_BEHIND );
            }
            CurHyperLabel->SetInteractive( m_pImpl->isInteractive() );
            m_pImpl->insertHyperLabel(_Index, CurHyperLabel );
        }
        else
        {
            CurHyperLabel = new HyperLabel(this);
            CurHyperLabel->SetInteractive( sal_False );
        }
        CurHyperLabel->SetHyperLabelPosition( RMENTRYPOINT_X, _YPos );
        CurHyperLabel->SetLabelAndSize( sLabel, m_pImpl->aHyperLabelPixelSize );
        CurHyperLabel->SetClickHdl(LINK( this, ORoadmap, ImplClickHdl ) );
        CurHyperLabel->SetID( _RMID );
        CurHyperLabel->SetIndex( _Index );
        if (!_bEnabled)
            CurHyperLabel->Enable( _bEnabled );
        return CurHyperLabel;
    }

    //---------------------------------------------------------------------
    void ORoadmap::SetRoadmapBitmap( const BitmapEx& _rBmp, sal_Bool _bInvalidate )
    {
        m_pImpl->setPicture( _rBmp );
        if ( _bInvalidate )
            Invalidate( );
    }

    //---------------------------------------------------------------------
    const BitmapEx& ORoadmap::GetRoadmapBitmap( ) const
    {
        return m_pImpl->getPicture( );
    }

    //---------------------------------------------------------------------
    void ORoadmap::SetRoadmapInteractive( sal_Bool _bInteractive )
    {
        m_pImpl->setInteractive( _bInteractive );
        HyperLabel* CurHyperLabel;
        HL_Vector pLocRoadmapItems = m_pImpl->getHyperLabels();
        for ( HL_Vector::iterator i = pLocRoadmapItems.begin(); i< pLocRoadmapItems.end(); i++)
        {
            CurHyperLabel = *i;
            CurHyperLabel->SetInteractive( _bInteractive );
        }
    }

    //---------------------------------------------------------------------
    sal_Bool ORoadmap::IsRoadmapInteractive()
    {
        return m_pImpl->isInteractive();
    }

    //---------------------------------------------------------------------
    void ORoadmap::SetRoadmapComplete( sal_Bool _bComplete )
    {
        sal_Bool OldbComplete = m_pImpl->isComplete();
        m_pImpl->setComplete( _bComplete);
        if (_bComplete)
        {
            if ( m_pImpl->InCompleteHyperLabel != NULL)
            {
                if (m_pImpl->getItemCount() > 0)
                {
                    HL_Vector pLocRoadmapItems = m_pImpl->getHyperLabels();
                    pLocRoadmapItems.pop_back();
                    delete m_pImpl->InCompleteHyperLabel;
                }
                m_pImpl->InCompleteHyperLabel = NULL;
            }
        }
        else if (OldbComplete)
            m_pImpl->InCompleteHyperLabel = InsertHyperLabel( m_pImpl->getItemCount(), ::String::CreateFromAscii("..."), RMINCOMPLETE );
    }

    //---------------------------------------------------------------------
    void ORoadmap::SetRoadmapLabel(HyperLabel* CurHyperLabel, sal_Int32 _nPrefix, String _sDescription)
    {
        xub_StrLen n_Pos = _sDescription.Search( String::CreateFromAscii(".") );
        String sID = ::String::CreateFromInt32( _nPrefix );
        _sDescription.Replace(0 , n_Pos, sID );
        CurHyperLabel->SetLabelAndSize( _sDescription, m_pImpl->aHyperLabelPixelSize );
    }

    //---------------------------------------------------------------------
    void ORoadmap::UpdatefollowingHyperLabels( ItemIndex _Index, sal_Int16 _n_add )
    {
        if ( _Index < ( m_pImpl->getItemCount() ) )
        {
            Point aPos;
            String sLabel;
            HL_Vector pLocRoadmapItems = m_pImpl->getHyperLabels();
            HyperLabel* CurHyperLabel = NULL;
            ItemIndex n_CurPrefix = _Index + 1 ;
            for ( HL_Vector::iterator i = pLocRoadmapItems.begin()+ _Index; i< pLocRoadmapItems.end(); i++)
            {
                CurHyperLabel = *i;
                aPos = CurHyperLabel->GetHyperLabelPosition();
                CurHyperLabel->SetIndex( n_CurPrefix - 1);
                sLabel = CurHyperLabel->GetLabel();
                SetRoadmapLabel(CurHyperLabel, n_CurPrefix, sLabel);
                CurHyperLabel->SetHyperLabelPosition( RMENTRYPOINT_X, aPos.Y() + (RMITEMDISTANCE_Y * _n_add ) );
                n_CurPrefix++;
            }
        }
        if ( ! m_pImpl->isComplete() )
        {
            sal_Int32 n_YPos = RMENTRYPOINT_Y + ( m_pImpl->getItemCount()) * RMITEMDISTANCE_Y ;
            m_pImpl->InCompleteHyperLabel->SetHyperLabelPosition( RMENTRYPOINT_X, n_YPos );
            m_pImpl->InCompleteHyperLabel->SetLabelAndSize( ConcatenateRMItemLabel( m_pImpl->getItemCount(), ::String::CreateFromAscii("...") ), m_pImpl->aHyperLabelPixelSize );
        }
    }

    //---------------------------------------------------------------------
    void ORoadmap::ReplaceRoadmapItem( ItemIndex _Index, ::rtl::OUString _RoadmapItem, ItemId _RMID, sal_Bool _bEnabled )
    {
        HyperLabel* CurHyperLabel = GetByIndex( _Index);
        if ( CurHyperLabel != NULL )
        {
            CurHyperLabel->SetLabelAndSize( ConcatenateRMItemLabel( _Index,  _RoadmapItem ), m_pImpl->aHyperLabelPixelSize );
            CurHyperLabel->SetID( _RMID );
            CurHyperLabel->Enable( _bEnabled );
        }
    }

    //---------------------------------------------------------------------
    RoadmapTypes::ItemIndex ORoadmap::GetItemCount() const
    {
        return m_pImpl->getItemCount();
    }

    //---------------------------------------------------------------------
    RoadmapTypes::ItemId ORoadmap::GetItemID( ItemIndex _nIndex ) const
    {
        const HyperLabel* pHyperLabel = GetByIndex( _nIndex );
        if ( pHyperLabel )
            return pHyperLabel->GetID();
        return -1;
    }

    //---------------------------------------------------------------------
    void ORoadmap::InsertRoadmapItem( ItemIndex _Index, ::rtl::OUString _RoadmapItem, ItemId _nUniqueId, sal_Bool _bEnabled )
    {
        HyperLabel* CurHyperLabel;
        CurHyperLabel = InsertHyperLabel(_Index, _RoadmapItem, _nUniqueId, _bEnabled);
            // Todo: YPos is superfluous, if items are always appended
        UpdatefollowingHyperLabels( _Index + 1);
    }

    //---------------------------------------------------------------------
    void ORoadmap::DeleteRoadmapItem( ItemIndex _Index )
    {
        if ( m_pImpl->getItemCount() > 0 && ( _Index > -1)  &&  ( _Index < m_pImpl->getItemCount() ) )
        {
            sal_Int32 n_YPos =  RMENTRYPOINT_Y + ( m_pImpl->getItemCount() * RMITEMDISTANCE_Y );
            m_pImpl->removeHyperLabel( _Index );
            UpdatefollowingHyperLabels( _Index, NREMOVERMITEM);
        }
    }

    //---------------------------------------------------------------------
    sal_Bool ORoadmap::IsRoadmapComplete( ) const
    {
        return m_pImpl->isComplete();
    }

    //---------------------------------------------------------------------
    sal_Bool ORoadmap::IsRoadmapItemEnabled( ItemId _nItemId ) const
    {
        const HyperLabel* _pLabelItem = GetByID( _nItemId );
        return _pLabelItem ? _pLabelItem->IsEnabled() : sal_False;
    }

    //---------------------------------------------------------------------
    void ORoadmap::EnableRoadmapItem( ItemId _nItemId, sal_Bool _bEnable )
    {
        HyperLabel* CurHyperLabel = GetByID( _nItemId );
        if ( CurHyperLabel != NULL )
            CurHyperLabel->Enable( _bEnable );
    }

    //---------------------------------------------------------------------
    void ORoadmap::ChangeRoadmapItemLabel( ItemId _nID, ::rtl::OUString _sLabel )
    {
        HyperLabel* CurHyperLabel = GetByID( _nID );
        if ( CurHyperLabel != NULL )
            CurHyperLabel->SetLabelAndSize( ConcatenateRMItemLabel( CurHyperLabel->GetIndex(), _sLabel ), m_pImpl->aHyperLabelPixelSize );
    }

    //---------------------------------------------------------------------
    void ORoadmap::ChangeRoadmapItemID( ItemId _nID, ItemId _NewID )
    {
        HyperLabel* CurHyperLabel = GetByID( _nID );
        if ( CurHyperLabel != NULL )
            CurHyperLabel->SetID( _NewID );
    }

    //---------------------------------------------------------------------
    HyperLabel* ORoadmap::GetByID( ItemId _nID)
    {
        HyperLabel* CurHyperLabel;
        ItemId nLocID = 0;
        HL_Vector &LocHyperLabels = m_pImpl->getHyperLabels();
        for ( HL_Vector::iterator i = LocHyperLabels.begin(); i< LocHyperLabels.end(); i++)
        {
            CurHyperLabel = *i;
            nLocID = CurHyperLabel->GetID();
            if ( nLocID == _nID )
                return CurHyperLabel;
        }
        return NULL;
    }

    //---------------------------------------------------------------------
    const HyperLabel* ORoadmap::GetByID( ItemId _nID ) const
    {
        return const_cast< ORoadmap* >( this )->GetByID( _nID );
    }

    //---------------------------------------------------------------------
    HyperLabel* ORoadmap::GetByIndex( ItemIndex _nItemIndex)
    {
        HL_Vector &LocHyperLabels = m_pImpl->getHyperLabels();
        if ((_nItemIndex > -1) && (_nItemIndex < m_pImpl->getItemCount( ) ) )
        {
            HyperLabel* CurHyperLabel = LocHyperLabels.at(_nItemIndex);    // Vectors are one-based
            return CurHyperLabel;
        }
        return NULL;
    }

    //---------------------------------------------------------------------
    const HyperLabel* ORoadmap::GetByIndex( ItemIndex _nItemIndex ) const
    {
        return const_cast< ORoadmap* >( this )->GetByIndex( _nItemIndex );
    }

    //---------------------------------------------------------------------
    RoadmapTypes::ItemId ORoadmap::GetNextAvailableItemId( ItemIndex _nNewIndex )
    {
        HyperLabel* CurHyperLabel;

        ItemIndex searchIndex = ++_nNewIndex;
        while ( searchIndex < m_pImpl->getItemCount() )
        {
            CurHyperLabel = GetByIndex( searchIndex );
            if ( CurHyperLabel->IsEnabled() )
                return CurHyperLabel->GetID( );

            ++searchIndex;
        }
        return -1;
    }

    //---------------------------------------------------------------------
    RoadmapTypes::ItemId ORoadmap::GetPreviousAvailableItemId( ItemIndex _nNewIndex )
    {
        HyperLabel* CurHyperLabel;
        ItemIndex searchIndex = --_nNewIndex;
        while ( searchIndex > -1 )
        {
            CurHyperLabel = GetByIndex( searchIndex );
            if ( CurHyperLabel->IsEnabled() )
                return CurHyperLabel->GetID( );

            searchIndex--;
        }
        return -1;
    }

    //---------------------------------------------------------------------
    void ORoadmap::DeselectOldRoadmapItems()
    {
        HL_Vector pLocRoadmapItems = m_pImpl->getHyperLabels();
        HyperLabel* CurHyperLabel = NULL;
        for ( HL_Vector::iterator i = pLocRoadmapItems.begin(); i< pLocRoadmapItems.end(); i++)
        {
            CurHyperLabel = *i;
            CurHyperLabel->ToggleBackgroundColor( COL_WHITE );
        }
    }

    //---------------------------------------------------------------------
    void ORoadmap::SetItemSelectHdl( const Link& _rHdl )
    {
        m_pImpl->setSelectHdl( _rHdl );
    }

    //---------------------------------------------------------------------
    Link ORoadmap::GetItemSelectHdl( ) const
    {
        return m_pImpl->getSelectHdl();
    }

    //---------------------------------------------------------------------
    void ORoadmap::Select()
    {
        GetItemSelectHdl().Call( this );
        CallEventListeners( VCLEVENT_ROADMAP_ITEMSELECTED );
    }

    //---------------------------------------------------------------------
    sal_Bool ORoadmap::SelectRoadmapItemByID( ItemId _nNewID )
    {
        DeselectOldRoadmapItems();
        HyperLabel* CurHyperLabel = GetByID( _nNewID );
        if (CurHyperLabel != NULL)
        {
            if (CurHyperLabel->IsEnabled())
            {
                CurHyperLabel->ToggleBackgroundColor( Color ( 194, 211, 238 ) );
                CurHyperLabel->GrabFocus();
                m_pImpl->setCurItemID(_nNewID);

                Select();
                return sal_True;
            }
        }
        return sal_False;
    }

    //---------------------------------------------------------------------
    void ORoadmap::Paint( const Rectangle& _rRect )
    {
        Control::Paint( _rRect );

        //.................................................................
        // draw the background
        {
            ColorChanger aColors( this, COL_TRANSPARENT, COL_WHITE );
            DrawRect( _rRect );
        }

        // the x pos for drawing our elements: starting at col 7 (counted in APPFONT units)
        sal_Int32 nElementXPos = LogicToPixel( Point( 7, 0 ), MAP_APPFONT ).X();

        //.................................................................
        // draw the bitmap
        if ( !!m_pImpl->getPicture() )
        {
            Size aBitmapSize = m_pImpl->getPicture().GetSizePixel();
            Size aMySize = GetOutputSizePixel();

            Point aBitmapPos( aMySize.Width() - aBitmapSize.Width(),  aMySize.Height() - aBitmapSize.Height() );

            // draw it
            DrawBitmapEx( aBitmapPos, m_pImpl->getPicture() );
        }

        //.................................................................
        // draw the headline
        DrawHeadline();
    }

    //---------------------------------------------------------------------
    void ORoadmap::DrawHeadline()
    {
        Point aTextPos = LogicToPixel( Point( RMENTRYPOINT_X, 8 ), MAP_APPFONT );

        Size aOutputSize( GetOutputSizePixel() );

        // draw it
        DrawText( Rectangle( aTextPos, aOutputSize ), GetText(), TEXT_DRAW_LEFT | TEXT_DRAW_TOP | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );

        DrawTextLine( aTextPos, aOutputSize.Width(), STRIKEOUT_NONE, UNDERLINE_SINGLE, sal_False );

    }

    //---------------------------------------------------------------------
    HyperLabel* ORoadmap::GetByPointer(Window* pWindow)
    {
        HyperLabel* CurHyperLabel;
        HL_Vector &LocHyperLabels = m_pImpl->getHyperLabels();
        for ( HL_Vector::iterator i = LocHyperLabels.begin(); i< LocHyperLabels.end(); i++)
        {
            CurHyperLabel = *i;
            if ( CurHyperLabel == pWindow )
                return CurHyperLabel;
        }
        return NULL;
    }

    //---------------------------------------------------------------------
    sal_Int32 ORoadmap::PreNotify( NotifyEvent& _rNEvt )
    {
        // capture KeyEvents for taskpane cycling
        if ( _rNEvt.GetType() == EVENT_KEYINPUT )
        {
            Window* pWindow = _rNEvt.GetWindow();
            HyperLabel* CurHyperLabel = GetByPointer( pWindow );
            if ( CurHyperLabel != NULL )
            {
                sal_Int16 nKeyCode = _rNEvt.GetKeyEvent()->GetKeyCode().GetCode();
                switch( nKeyCode )
                {
                    case KEY_UP:
                        {   // Note: Performancewhise this is not optimal, because we search for an ID in the labels
                            //       and afterwards we search again for a label with the appropriate ID ->
                            //       unnecessarily we search twice!!!
                            ItemId nPrevItemID = GetPreviousAvailableItemId( CurHyperLabel->GetIndex() );
                            if ( nPrevItemID != -1 )
                                return SelectRoadmapItemByID( nPrevItemID );
                        }
                        break;
                    case KEY_DOWN:
                        {
                            ItemId nNextItemID = GetNextAvailableItemId( CurHyperLabel->GetIndex() );
                            if ( nNextItemID != -1 )
                                return SelectRoadmapItemByID( nNextItemID );
                        }
                        break;
                    case KEY_SPACE:
                        return SelectRoadmapItemByID( CurHyperLabel->GetID() );
                }
            }
        }
        return Window::PreNotify( _rNEvt );
    }

    //---------------------------------------------------------------------
      IMPL_LINK(ORoadmap, ImplClickHdl, HyperLabel*, _CurHyperLabel)
    {
       return SelectRoadmapItemByID( _CurHyperLabel->GetID() );
    }

//.........................................................................
}   // namespace svt
//.........................................................................
