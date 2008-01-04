/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: roadmap.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-04 16:25:40 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
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
#include <memory>

#define RMENTRYPOINT_X      4
#define RMENTRYPOINT_Y      27
#define RMITEMDISTANCE_Y    6
#define RMINCOMPLETE        -1
#define NREMOVERMITEM       -1
#define NADDITEM            1
#define INCOMPLETELABEL     ::String::CreateFromAscii("...")        // TODO: Cast to String

//.........................................................................
namespace svt
{
//.........................................................................

    typedef std::vector< ::rtl::OUString > S_Vector;
    typedef std::vector< ORoadmapHyperLabel* > HL_Vector;

    //=====================================================================
    //= FontChanger
    //=====================================================================
//  class FontChanger
//  {
//  protected:
//      OutputDevice*   m_pDev;
//
//  public:
//      FontChanger( OutputDevice* _pDev, const Font& _rNewFont )
//          :m_pDev( _pDev )
//      {
//          m_pDev->Push( PUSH_FONT );
//          m_pDev->SetFont( _rNewFont );
//      }
//
//      ~FontChanger()
//      {
//          m_pDev->Pop( );
//      }
//  };


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
        RoadmapImpl() :
            m_bInteractive( sal_True ),
            m_bComplete( sal_True ) {}

        Size aHyperLabelPixelSize;
        ORoadmapHyperLabel* InCompleteHyperLabel;

        void                addHyperLabel( ORoadmapHyperLabel*  _rRoadmapStep ) { m_aRoadmapSteps.push_back(_rRoadmapStep); }

        HL_Vector&          getHyperLabels() { return m_aRoadmapSteps; }
        const HL_Vector&    getHyperLabels() const { return m_aRoadmapSteps; }

        void                insertHyperLabel( ItemIndex _Index, ORoadmapHyperLabel* _rRoadmapStep ) { m_aRoadmapSteps.insert( m_aRoadmapSteps.begin() + _Index, _rRoadmapStep ); }

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
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        Color aTextColor = rStyleSettings.GetFieldTextColor();
        Font aFont = GetFont( );
        aFont.SetColor( aTextColor );
        aFont.SetWeight( WEIGHT_BOLD );
        aFont.SetUnderline( UNDERLINE_SINGLE );
        SetFont( aFont );
        SetBackground( Wallpaper( rStyleSettings.GetFieldColor() ) );
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
        m_pImpl->getHyperLabels().clear();
        for ( HL_Vector::iterator i = pLocRoadmapItems.begin(); i< pLocRoadmapItems.end(); i++)
        {
            delete *i;
        }
        if ( ! m_pImpl->isComplete() )
            delete m_pImpl->InCompleteHyperLabel;
        delete m_pImpl;
        m_pImpl = NULL;
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
        aLogicSize.Width() -= (2*RMENTRYPOINT_X);
        m_pImpl->aHyperLabelPixelSize = LogicToPixel( aLogicSize, MAP_APPFONT );
        EnableMapMode( sal_False );
    }


    ORoadmapHyperLabel* ORoadmap::GetPreviousHyperLabel( ItemIndex _Index)
    {
        ORoadmapHyperLabel* OldHyperLabel = NULL;
        if (_Index > 0)
            OldHyperLabel = m_pImpl->getHyperLabels().at( _Index - 1 );
        return OldHyperLabel;
    }


       //---------------------------------------------------------------------

    ORoadmapHyperLabel* ORoadmap::InsertHyperLabel( ItemIndex _Index, ::rtl::OUString _sLabel, ItemId _RMID, sal_Bool _bEnabled)
    {
        if (m_pImpl->getItemCount() == 0 )
            InitializeHyperLabelSize();
        ORoadmapHyperLabel* CurHyperLabel;
        ORoadmapHyperLabel* OldHyperLabel = GetPreviousHyperLabel( _Index);

        if (_RMID != RMINCOMPLETE )
        {
            CurHyperLabel = new ORoadmapHyperLabel(this, WB_WORDBREAK);
            CurHyperLabel->SetInteractive( m_pImpl->isInteractive() );
            m_pImpl->insertHyperLabel(_Index, CurHyperLabel );
        }
        else
        {
            CurHyperLabel = new ORoadmapHyperLabel(this);
            CurHyperLabel->SetInteractive( sal_False );
        }
        CurHyperLabel->SetPosition( OldHyperLabel );
        CurHyperLabel->SetLabelAndSize( _Index, _sLabel, m_pImpl->aHyperLabelPixelSize );
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
        ORoadmapHyperLabel* CurHyperLabel;
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
    void ORoadmap::SetRoadmapLabel(ORoadmapHyperLabel* CurHyperLabel, sal_Int32 _nPrefix, String _sDescription)
    {
        xub_StrLen n_Pos = _sDescription.Search( String::CreateFromAscii(".") );
        String sID = ::String::CreateFromInt32( _nPrefix );
        _sDescription.Replace(0 , n_Pos, sID );
        CurHyperLabel->SetLabelAndSize( _nPrefix, _sDescription, m_pImpl->aHyperLabelPixelSize );
    }

    //---------------------------------------------------------------------
    void ORoadmap::UpdatefollowingHyperLabels( ItemIndex _Index, sal_Int16 )
    {
        if ( _Index < ( m_pImpl->getItemCount() ) )
        {
            Point aPos;
            String sLabel;
            HL_Vector pLocRoadmapItems = m_pImpl->getHyperLabels();
            ORoadmapHyperLabel* CurHyperLabel = NULL;
            ItemIndex n_CurPrefix = _Index + 1 ;
            for ( HL_Vector::iterator i = pLocRoadmapItems.begin()+ _Index; i< pLocRoadmapItems.end(); i++)
            {
                CurHyperLabel = *i;
                aPos = CurHyperLabel->GetLogicalPosition();
                CurHyperLabel->SetIndex( n_CurPrefix - 1);
                sLabel = CurHyperLabel->GetLabel();
                SetRoadmapLabel(CurHyperLabel, n_CurPrefix, sLabel);
                ORoadmapHyperLabel* OldHyperLabel = GetPreviousHyperLabel( n_CurPrefix-1);
                CurHyperLabel->SetPosition( OldHyperLabel);
                n_CurPrefix++;
            }
        }
        if ( ! m_pImpl->isComplete() )
        {
            ORoadmapHyperLabel* OldHyperLabel = GetPreviousHyperLabel( m_pImpl->getItemCount());
            m_pImpl->InCompleteHyperLabel->SetPosition( OldHyperLabel );
            m_pImpl->InCompleteHyperLabel->SetLabelAndSize( m_pImpl->getItemCount(), ::String::CreateFromAscii("..."), m_pImpl->aHyperLabelPixelSize );
        }
    }

    //---------------------------------------------------------------------
    void ORoadmap::ReplaceRoadmapItem( ItemIndex _Index, ::rtl::OUString _RoadmapItem, ItemId _RMID, sal_Bool _bEnabled )
    {
        ORoadmapHyperLabel* CurHyperLabel = GetByIndex( _Index);
        if ( CurHyperLabel != NULL )
        {
            CurHyperLabel->SetLabelAndSize( _Index,  _RoadmapItem, m_pImpl->aHyperLabelPixelSize );
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
        const ORoadmapHyperLabel* pHyperLabel = GetByIndex( _nIndex );
        if ( pHyperLabel )
            return pHyperLabel->GetID();
        return -1;
    }

    //---------------------------------------------------------------------
    RoadmapTypes::ItemIndex ORoadmap::GetItemIndex( ItemId _nID ) const
    {
        ORoadmapHyperLabel* CurHyperLabel;
        ItemId nLocID = 0;
        HL_Vector &LocHyperLabels = m_pImpl->getHyperLabels();
        ItemIndex nResult = 0;
        for ( HL_Vector::iterator i = LocHyperLabels.begin(); i< LocHyperLabels.end(); i++)
        {
            CurHyperLabel = *i;
            nLocID = CurHyperLabel->GetID();
            if ( nLocID == _nID )
                return nResult;
            nResult++;
        }
        return -1;
    }

    //---------------------------------------------------------------------
    void ORoadmap::InsertRoadmapItem( ItemIndex _Index, ::rtl::OUString _RoadmapItem, ItemId _nUniqueId, sal_Bool _bEnabled )
    {
        ORoadmapHyperLabel* CurHyperLabel;
        CurHyperLabel = InsertHyperLabel(_Index, _RoadmapItem, _nUniqueId, _bEnabled);
            // Todo: YPos is superfluous, if items are always appended
        UpdatefollowingHyperLabels( _Index + 1);
    }

    //---------------------------------------------------------------------
    void ORoadmap::DeleteRoadmapItem( ItemIndex _Index )
    {
        if ( m_pImpl->getItemCount() > 0 && ( _Index > -1)  &&  ( _Index < m_pImpl->getItemCount() ) )
        {
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
    sal_Bool ORoadmap::IsRoadmapItemEnabled( ItemId _nItemId, ItemIndex _nStartIndex  ) const
    {
        const ORoadmapHyperLabel* _pLabelItem = GetByID( _nItemId, _nStartIndex  );
        return _pLabelItem ? _pLabelItem->IsEnabled() : sal_False;
    }

    //---------------------------------------------------------------------
    void ORoadmap::EnableRoadmapItem( ItemId _nItemId, sal_Bool _bEnable, ItemIndex _nStartIndex )
    {
        ORoadmapHyperLabel* CurHyperLabel = GetByID( _nItemId, _nStartIndex );
        if ( CurHyperLabel != NULL )
            CurHyperLabel->Enable( _bEnable );
    }

    //---------------------------------------------------------------------
    void ORoadmap::ChangeRoadmapItemLabel( ItemId _nID, ::rtl::OUString _sLabel, ItemIndex _nStartIndex )
    {
        ORoadmapHyperLabel* CurHyperLabel = GetByID( _nID, _nStartIndex );
        if ( CurHyperLabel != NULL )
        {
            CurHyperLabel->SetLabelAndSize( CurHyperLabel->GetIndex(), _sLabel, m_pImpl->aHyperLabelPixelSize );
            HL_Vector pLocRoadmapItems = m_pImpl->getHyperLabels();
            ItemIndex Index = _nStartIndex;
            for ( HL_Vector::iterator i = pLocRoadmapItems.begin()+ Index; i< pLocRoadmapItems.end(); i++)
            {
                CurHyperLabel = *i;
                ORoadmapHyperLabel* OldHyperLabel = GetPreviousHyperLabel( Index );
                CurHyperLabel->SetPosition( OldHyperLabel);
                Index++;
            }
        }
    }

    //---------------------------------------------------------------------

    ::rtl::OUString ORoadmap::GetRoadmapItemLabel( ItemId _nID, ItemIndex _nStartIndex )
    {
        ORoadmapHyperLabel* CurHyperLabel = GetByID( _nID, _nStartIndex );
        if ( CurHyperLabel != NULL )
            return CurHyperLabel->GetLabel();
        else
            return ::rtl::OUString();
    }

    //---------------------------------------------------------------------
    void ORoadmap::ChangeRoadmapItemID( ItemId _nID, ItemId _NewID, ItemIndex _nStartIndex )
    {
        ORoadmapHyperLabel* CurHyperLabel = GetByID( _nID, _nStartIndex );
        if ( CurHyperLabel != NULL )
            CurHyperLabel->SetID( _NewID );
    }

    //---------------------------------------------------------------------
    ORoadmapHyperLabel* ORoadmap::GetByID( ItemId _nID, ItemIndex _nStartIndex)
    {
        ORoadmapHyperLabel* CurHyperLabel;
        ItemId nLocID = 0;
        HL_Vector &LocHyperLabels = m_pImpl->getHyperLabels();
        for ( HL_Vector::iterator i = LocHyperLabels.begin()+ _nStartIndex; i< LocHyperLabels.end(); i++)
        {
            CurHyperLabel = *i;
            nLocID = CurHyperLabel->GetID();
            if ( nLocID == _nID )
                return CurHyperLabel;
        }
        return NULL;
    }

    //---------------------------------------------------------------------
    const ORoadmapHyperLabel* ORoadmap::GetByID( ItemId _nID, ItemIndex _nStartIndex  ) const
    {
        return const_cast< ORoadmap* >( this )->GetByID( _nID, _nStartIndex );
    }

    //---------------------------------------------------------------------
    ORoadmapHyperLabel* ORoadmap::GetByIndex( ItemIndex _nItemIndex)
    {
        HL_Vector &LocHyperLabels = m_pImpl->getHyperLabels();
        if ((_nItemIndex > -1) && (_nItemIndex < m_pImpl->getItemCount( ) ) )
        {
            ORoadmapHyperLabel* CurHyperLabel = LocHyperLabels.at(_nItemIndex);    // Vectors are one-based
            return CurHyperLabel;
        }
        return NULL;
    }

    //---------------------------------------------------------------------
    const ORoadmapHyperLabel* ORoadmap::GetByIndex( ItemIndex _nItemIndex ) const
    {
        return const_cast< ORoadmap* >( this )->GetByIndex( _nItemIndex );
    }

    //---------------------------------------------------------------------
    RoadmapTypes::ItemId ORoadmap::GetNextAvailableItemId( ItemIndex _nNewIndex )
    {
        ORoadmapHyperLabel* CurHyperLabel;

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
        ORoadmapHyperLabel* CurHyperLabel;
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
        ORoadmapHyperLabel* CurHyperLabel = NULL;
        for ( HL_Vector::iterator i = pLocRoadmapItems.begin(); i< pLocRoadmapItems.end(); i++)
        {
            CurHyperLabel = *i;
            CurHyperLabel->ToggleBackgroundColor( COL_TRANSPARENT );
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
    void ORoadmap::GetFocus()
    {
        ORoadmapHyperLabel* pCurHyperLabel = GetByID( GetCurrentRoadmapItemID() );
        if ( pCurHyperLabel != NULL )
            pCurHyperLabel->GrabFocus();
    }

    //---------------------------------------------------------------------
    sal_Bool ORoadmap::SelectRoadmapItemByID( ItemId _nNewID )
    {
        DeselectOldRoadmapItems();
        ORoadmapHyperLabel* CurHyperLabel = GetByID( _nNewID );
        if (CurHyperLabel != NULL)
        {
            if (CurHyperLabel->IsEnabled())
            {
                const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
                CurHyperLabel->ToggleBackgroundColor( rStyleSettings.GetHighlightColor() ); //HighlightColor

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
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        SetLineColor( rStyleSettings.GetFieldTextColor());
        SetTextColor(rStyleSettings.GetFieldTextColor());
    }

    //---------------------------------------------------------------------
    ORoadmapHyperLabel* ORoadmap::GetByPointer(Window* pWindow)
    {
        ORoadmapHyperLabel* CurHyperLabel;
        HL_Vector &LocHyperLabels = m_pImpl->getHyperLabels();
        for ( HL_Vector::iterator i = LocHyperLabels.begin(); i< LocHyperLabels.end(); i++)
        {
            CurHyperLabel = *i;
            if ( (CurHyperLabel->GetIDLabel() == pWindow) || (CurHyperLabel->GetDescriptionHyperLabel() == pWindow) )
                return CurHyperLabel;
        }
        return NULL;
    }

    //---------------------------------------------------------------------
    long ORoadmap::PreNotify( NotifyEvent& _rNEvt )
    {
        // capture KeyEvents for taskpane cycling
        if ( _rNEvt.GetType() == EVENT_KEYINPUT )
        {
            Window* pWindow = _rNEvt.GetWindow();
            ORoadmapHyperLabel* CurHyperLabel = GetByPointer( pWindow );
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



    void ORoadmap::DataChanged( const DataChangedEvent& rDCEvt )
    {
        if ((( rDCEvt.GetType() == DATACHANGED_SETTINGS )   ||
            ( rDCEvt.GetType() == DATACHANGED_DISPLAY   ))  &&
            ( rDCEvt.GetFlags() & SETTINGS_STYLE        ))
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            SetBackground( Wallpaper( rStyleSettings.GetFieldColor() ) );
            Color aTextColor = rStyleSettings.GetFieldTextColor();
            Font aFont = GetFont();
            aFont.SetColor( aTextColor );
            SetFont( aFont );
            RoadmapTypes::ItemId curItemID = GetCurrentRoadmapItemID();
            ORoadmapHyperLabel* pLabelItem = GetByID( curItemID );
            pLabelItem->ToggleBackgroundColor(rStyleSettings.GetHighlightColor());
            Invalidate();
        }
    }


    ORoadmapHyperLabel::ORoadmapHyperLabel( Window* _pParent, const ResId& )
    {
        mpIDLabel = new ORoadmapIDHyperLabel(_pParent, WB_WORDBREAK);
        mpDescHyperLabel = new HyperLabel(_pParent, WB_TABSTOP | WB_WORDBREAK);
    }


    ORoadmapHyperLabel::ORoadmapHyperLabel( Window* _pParent, WinBits )
    {
        mpIDLabel = new ORoadmapIDHyperLabel(_pParent, WB_WORDBREAK);
        mpIDLabel->SetTextColor( mpIDLabel->GetSettings().GetStyleSettings().GetFieldTextColor( ) );
        mpDescHyperLabel = new HyperLabel(_pParent, WB_TABSTOP | WB_WORDBREAK);
    }

    //---------------------------------------------------------------------
    void ORoadmapHyperLabel::GrabFocus()
    {
        if ( mpDescHyperLabel )
            mpDescHyperLabel->GrabFocus();
    }


    void ORoadmapHyperLabel::SetInteractive( sal_Bool _bInteractive )
    {
        if ( mpDescHyperLabel )
        mpDescHyperLabel->SetInteractive(_bInteractive);
    }

    void ORoadmapHyperLabel::SetID( sal_Int16 _ID )
    {
        if ( mpDescHyperLabel )
            mpDescHyperLabel->SetID(_ID);
    }

    sal_Int16 ORoadmapHyperLabel::GetID() const
    {
        return mpDescHyperLabel ? mpDescHyperLabel->GetID() : sal_Int16(-1);
    }

    void ORoadmapHyperLabel::SetIndex( sal_Int32 _Index )
    {
        if ( mpDescHyperLabel )
            mpDescHyperLabel->SetIndex(_Index);
    }


    sal_Int32 ORoadmapHyperLabel::GetIndex() const
    {
        return mpDescHyperLabel ? mpDescHyperLabel->GetIndex() : sal_Int32(-1);
    }


    void ORoadmapHyperLabel::SetLabel( ::rtl::OUString _rText )
    {
        if ( mpDescHyperLabel )
            mpDescHyperLabel->SetText(_rText);
    }


    ::rtl::OUString ORoadmapHyperLabel::GetLabel( )
    {
            return mpDescHyperLabel ? mpDescHyperLabel->GetText() : String();
    }


    void ORoadmapHyperLabel::SetPosition(ORoadmapHyperLabel* OldHyperLabel)
    {
        Point aNewLogicalPoint;
        Point aNewPoint;
        if (OldHyperLabel == NULL)
        {
            aNewLogicalPoint = Point( RMENTRYPOINT_X, RMENTRYPOINT_Y);
            aNewPoint = mpIDLabel->LogicToPixel(aNewLogicalPoint, MAP_APPFONT );
        }
        else
        {
            Size aOldSize = OldHyperLabel->GetDescriptionHyperLabel()->GetSizePixel();
            Point aOldLogicalPoint = OldHyperLabel->GetLogicalPosition();
            aNewLogicalPoint = Point(aOldLogicalPoint.X(), (aOldLogicalPoint.Y() + RMITEMDISTANCE_Y));
            aNewPoint = mpIDLabel->LogicToPixel(aNewLogicalPoint, MAP_APPFONT );
            aNewPoint = Point(aNewPoint.X(),aNewPoint.Y() + aOldSize.Height());
        }
        mpIDLabel->SetPosPixel( aNewPoint );
        sal_Int32 xDescPos = aNewPoint.X() + mpIDLabel->GetSizePixel().Width();
        mpDescHyperLabel->SetPosPixel( Point(xDescPos, aNewPoint.Y()) );
    }


    void ORoadmapHyperLabel::SetZOrder( ORoadmapHyperLabel* pRefRoadmapHyperLabel, USHORT nFlags )
    {
        if (pRefRoadmapHyperLabel == NULL)
            mpDescHyperLabel->SetZOrder( NULL, nFlags); //WINDOW_ZORDER_FIRST );
        else
            mpDescHyperLabel->SetZOrder( pRefRoadmapHyperLabel->mpDescHyperLabel, nFlags); //, WINDOW_ZORDER_BEHIND );
    }


    void ORoadmapHyperLabel::Enable( BOOL _bEnable)
    {
        mpIDLabel->Enable(_bEnable);
        mpDescHyperLabel->Enable(_bEnable);
    }

    BOOL ORoadmapHyperLabel::IsEnabled() const
    {
        return mpIDLabel->IsEnabled();
    }

//  void ORoadmapHyperLabel::GrabFocus()
//  {
//      mpDescHyperLabel->GrabFocus();
//
//  }

    void ORoadmapHyperLabel::ToggleBackgroundColor( const Color& _rGBColor )
    {
        if (_rGBColor == COL_TRANSPARENT)
        {
            mpIDLabel->SetTextColor( mpIDLabel->GetSettings().GetStyleSettings().GetFieldTextColor( ) );
            mpIDLabel->SetControlBackground( COL_TRANSPARENT );
        }
        else
        {
            mpIDLabel->SetControlBackground( mpIDLabel->GetSettings().GetStyleSettings().GetHighlightColor() );
            mpIDLabel->SetTextColor( mpIDLabel->GetSettings().GetStyleSettings().GetHighlightTextColor( ) );
        }
        mpDescHyperLabel->ToggleBackgroundColor(_rGBColor);
    }


    Point ORoadmapHyperLabel::GetLogicalPosition()
    {
        Point aPoint = mpIDLabel->GetPosPixel( );
        Size aSize = Size(aPoint.X(), aPoint.Y());
        aSize = mpIDLabel->PixelToLogic( aSize, MAP_APPFONT );
        aPoint = Point(aSize.Width(), aSize.Height());
        return aPoint;
    }


    void ORoadmapHyperLabel::SetLabelAndSize( ItemIndex _RMIndex, ::rtl::OUString _rText, const Size& _rNewSize)
    {
         Size rIDSize = _rNewSize;
         ::rtl::OUString aStr = ::rtl::OUString::valueOf( (sal_Int32)( _RMIndex + 1 ) ) +  ::rtl::OUString::createFromAscii( "." );
        rIDSize.Width() = (sal_Int32) mpIDLabel->GetTextWidth( aStr );
        long nMaxWidth = mpIDLabel->GetTextWidth( ::rtl::OUString::createFromAscii("100.") );
        rIDSize.Width() = ::std::min( rIDSize.getWidth(),nMaxWidth );
         mpIDLabel->SetSizePixel(mpIDLabel->LogicToPixel( rIDSize));
         mpIDLabel->SetText( aStr);
         mpIDLabel->Show();
        Size rDescSize = _rNewSize;
        rDescSize.Width() -= rIDSize.Width();
        sal_Int32 xDescPos = mpIDLabel->GetPosPixel().X() + mpIDLabel->GetSizePixel().Width();
        sal_Int32 yDescPos = mpIDLabel->GetPosPixel().Y();
        Point aPoint = Point(xDescPos, yDescPos);
        mpDescHyperLabel->SetPosPixel( aPoint );
        mpDescHyperLabel->SetLabelAndSize(_rText, rDescSize);
        mpIDLabel->SetSizePixel( Size( mpIDLabel->GetSizePixel().Width(), mpDescHyperLabel->GetSizePixel().Height() ) );
    }

    ORoadmapHyperLabel::~ORoadmapHyperLabel( )
    {
        {
            ::std::auto_ptr<Control> aTemp(mpIDLabel);
            mpIDLabel = NULL;
        }
        {
            ::std::auto_ptr<Control> aTemp(mpDescHyperLabel);
            mpDescHyperLabel = NULL;
        }
    }


    void ORoadmapHyperLabel::SetClickHdl( const Link& rLink )
    {
        if ( mpDescHyperLabel )
            mpDescHyperLabel->SetClickHdl( rLink);
    }

    const Link& ORoadmapHyperLabel::GetClickHdl( ) const
    {
        return mpDescHyperLabel->GetClickHdl();
    }


    ORoadmapIDHyperLabel::ORoadmapIDHyperLabel( Window* _pParent, const ResId& _rId )
        :FixedText( _pParent, _rId )
    {
    }

    ORoadmapIDHyperLabel::ORoadmapIDHyperLabel( Window* _pParent, WinBits _nWinStyle )
        :FixedText( _pParent, _nWinStyle )
    {

    }


    ORoadmapIDHyperLabel::~ORoadmapIDHyperLabel( )
    {
    }


    void ORoadmapIDHyperLabel::DataChanged( const DataChangedEvent& rDCEvt )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        FixedText::DataChanged( rDCEvt );
        if ((( rDCEvt.GetType() == DATACHANGED_SETTINGS )   ||
            ( rDCEvt.GetType() == DATACHANGED_DISPLAY   ))  &&
            ( rDCEvt.GetFlags() & SETTINGS_STYLE        ))
        {
            const Color& rGBColor = GetControlBackground();
            if (rGBColor == COL_TRANSPARENT)
                SetTextColor( rStyleSettings.GetFieldTextColor( ) );
            else
            {
                SetControlBackground(rStyleSettings.GetHighlightColor());
                SetTextColor( rStyleSettings.GetHighlightTextColor( ) );
            }
            Invalidate();
        }
    }




//.........................................................................
}   // namespace svt
//.........................................................................
