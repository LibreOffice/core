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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

//------------------------------------------------------------------------
//
// Global header
//
//------------------------------------------------------------------------
#include <svl/itemset.hxx>
#include <editeng/editdata.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpool.hxx>

//------------------------------------------------------------------------
//
// Project-local header
//
//------------------------------------------------------------------------

#include "AccessibleEmptyEditSource.hxx"
#include <svx/unoshtxt.hxx>
#include <svx/globaldrawitempool.hxx>

namespace accessibility
{

    /** This class simply wraps a SvxTextEditSource, forwarding all
        methods except the GetBroadcaster() call
     */
    class AccessibleProxyEditSource_Impl : public SvxEditSource
    {
    public:
        /** Construct AccessibleEmptyEditSource_Impl

            @param rBrdCast

            Proxy broadcaster to allow seamless flipping of edit source implementations. ProxyEditSource and EmptyEditSource
         */
        AccessibleProxyEditSource_Impl( SdrObject&      rObj,
                                        SdrView&        rView,
                                        const Window&   rViewWindow );
        ~AccessibleProxyEditSource_Impl();

        // from the SvxEditSource interface
        SvxTextForwarder*       GetTextForwarder();
        SvxViewForwarder*       GetViewForwarder();
        SvxEditViewForwarder*   GetEditViewForwarder( sal_Bool bCreate = sal_False );

        SvxEditSource*          Clone() const;

        void                    UpdateData();

        SfxBroadcaster&         GetBroadcaster() const;

    private:
        SvxTextEditSource       maEditSource;

    };

    /** Dummy class, faking exactly one empty paragraph for EditEngine accessibility
     */
    class AccessibleEmptyEditSource_Impl : public SvxEditSource, public SvxViewForwarder, public SvxTextForwarder, public SfxBroadcaster
    {
    public:

        AccessibleEmptyEditSource_Impl() {}
        ~AccessibleEmptyEditSource_Impl() {}

        // from the SfxListener interface
        void                    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

        // SvxEditSource
        SvxTextForwarder*       GetTextForwarder() { return this; }
        SvxViewForwarder*       GetViewForwarder() { return this; }
        SvxEditSource*          Clone() const { return NULL; }
        void                    UpdateData() {}
        SfxBroadcaster&         GetBroadcaster() const { return *(const_cast<AccessibleEmptyEditSource_Impl*>(this)); }

        // SvxTextForwarder
        sal_uInt16          GetParagraphCount() const { return 1; }
        sal_uInt16          GetTextLen( sal_uInt16 /*nParagraph*/ ) const { return 0; }
        String          GetText( const ESelection& /*rSel*/ ) const { return String(); }
        SfxItemSet      GetAttribs( const ESelection& /*rSel*/, sal_Bool /*bOnlyHardAttrib*/ = 0 ) const
        {
            // AW: Very dangerous: The former implementation used a SfxItemPool created on the
            // fly which of course was deleted again ASAP. Thus, the returned SfxItemSet was using
            // a deleted Pool by design.
            return SfxItemSet(GetGlobalDrawObjectItemPool());
        }
        SfxItemSet      GetParaAttribs( sal_uInt16 /*nPara*/ ) const { return GetAttribs(ESelection()); }
        void            SetParaAttribs( sal_uInt16 /*nPara*/, const SfxItemSet& /*rSet*/ ) {}
        void            RemoveAttribs( const ESelection& /*rSelection*/, sal_Bool /*bRemoveParaAttribs*/, sal_uInt16 /*nWhich*/ ){}
        void            GetPortions( sal_uInt16 /*nPara*/, SvUShorts& /*rList*/ ) const {}

        sal_uInt16          GetItemState( const ESelection& /*rSel*/, sal_uInt16 /*nWhich*/ ) const { return 0; }
        sal_uInt16          GetItemState( sal_uInt16 /*nPara*/, sal_uInt16 /*nWhich*/ ) const { return 0; }

        SfxItemPool*    GetPool() const { return NULL; }

        void            QuickInsertText( const String& /*rText*/, const ESelection& /*rSel*/ ) {}
        void            QuickInsertField( const SvxFieldItem& /*rFld*/, const ESelection& /*rSel*/ ) {}
        void            QuickSetAttribs( const SfxItemSet& /*rSet*/, const ESelection& /*rSel*/ ) {}
        void            QuickInsertLineBreak( const ESelection& /*rSel*/ ) {}

        const SfxItemSet * GetEmptyItemSetPtr() { return 0; }

        void        AppendParagraph() {}
        xub_StrLen  AppendTextPortion( sal_uInt16 /*nPara*/, const String & /*rText*/, const SfxItemSet & /*rSet*/ ) { return 0; }

        //XTextCopy
        void        CopyText(const SvxTextForwarder& ){}

        XubString       CalcFieldValue( const SvxFieldItem& /*rField*/, sal_uInt16 /*nPara*/, sal_uInt16 /*nPos*/, Color*& /*rpTxtColor*/, Color*& /*rpFldColor*/ )
        {
            return  XubString();
        }
        void            FieldClicked( const SvxFieldItem&, sal_uInt16, xub_StrLen ) {;}

        sal_Bool            IsValid() const { return sal_True; }

        void            SetNotifyHdl( const Link& ) {}
        LanguageType    GetLanguage( sal_uInt16, sal_uInt16 ) const { return LANGUAGE_DONTKNOW; }
        sal_uInt16          GetFieldCount( sal_uInt16 ) const { return 0; }
        EFieldInfo      GetFieldInfo( sal_uInt16, sal_uInt16 ) const { return EFieldInfo(); }
        EBulletInfo     GetBulletInfo( sal_uInt16 ) const { return EBulletInfo(); }
        Rectangle       GetCharBounds( sal_uInt16, sal_uInt16 ) const { return Rectangle(); }
        Rectangle       GetParaBounds( sal_uInt16 ) const { return Rectangle(); }
        MapMode         GetMapMode() const { return MapMode(); }
        OutputDevice*   GetRefDevice() const { return NULL; }
        sal_Bool        GetIndexAtPoint( const Point&, sal_uInt16&, sal_uInt16& ) const { return sal_False; }
        sal_Bool        GetWordIndices( sal_uInt16, sal_uInt16, sal_uInt16&, sal_uInt16& ) const { return sal_False; }
    sal_Bool        GetAttributeRun( sal_uInt16&, sal_uInt16&, sal_uInt16, sal_uInt16, sal_Bool) const { return sal_False; }
        sal_uInt16          GetLineCount( sal_uInt16 nPara ) const { return nPara == 0 ? 1 : 0; }
        sal_uInt16          GetLineLen( sal_uInt16, sal_uInt16 ) const { return 0; }
        void            GetLineBoundaries( /*out*/sal_uInt16 & rStart, /*out*/sal_uInt16 & rEnd, sal_uInt16 /*nParagraph*/, sal_uInt16 /*nLine*/ ) const  { rStart = rEnd = 0; }
        sal_uInt16          GetLineNumberAtIndex( sal_uInt16 /*nPara*/, sal_uInt16 /*nIndex*/ ) const   { return 0; }

        // the following two methods would, strictly speaking, require
        // a switch to a real EditSource, too. Fortunately, the
        // AccessibleEditableTextPara implementation currently always
        // calls GetEditViewForwarder(true) before doing
        // changes. Thus, we rely on this behabviour here (problem
        // when that changes: via accessibility API, it would no
        // longer be possible to enter text in previously empty
        // shapes).
        sal_Bool        Delete( const ESelection& ) { return sal_False; }
        sal_Bool        InsertText( const String&, const ESelection& ) { return sal_False; }
        sal_Bool        QuickFormatDoc( sal_Bool ) { return sal_True; }
        sal_Int16       GetDepth( sal_uInt16 ) const { return -1; }
        sal_Bool        SetDepth( sal_uInt16, sal_Int16 ) { return sal_True; }

        Rectangle       GetVisArea() const { return Rectangle(); }
        Point           LogicToPixel( const Point& rPoint, const MapMode& /*rMapMode*/ ) const { return rPoint; }
        Point           PixelToLogic( const Point& rPoint, const MapMode& /*rMapMode*/ ) const { return rPoint; }

    };

    // -------------------------------------------------------------------------
    // Implementing AccessibleProxyEditSource_Impl
    // -------------------------------------------------------------------------

    AccessibleProxyEditSource_Impl::AccessibleProxyEditSource_Impl( SdrObject&      rObj,
                                                                    SdrView&        rView,
                                                                    const Window&   rViewWindow ) :
        maEditSource( rObj, 0, rView, rViewWindow )
    {
    }

    AccessibleProxyEditSource_Impl::~AccessibleProxyEditSource_Impl()
    {
    }

    SvxTextForwarder* AccessibleProxyEditSource_Impl::GetTextForwarder()
    {
        return maEditSource.GetTextForwarder();
    }

    SvxViewForwarder* AccessibleProxyEditSource_Impl::GetViewForwarder()
    {
        return maEditSource.GetViewForwarder();
    }

    SvxEditViewForwarder* AccessibleProxyEditSource_Impl::GetEditViewForwarder( sal_Bool bCreate )
    {
        return maEditSource.GetEditViewForwarder( bCreate );
    }

    SvxEditSource* AccessibleProxyEditSource_Impl::Clone() const
    {
        return maEditSource.Clone();
    }

    void AccessibleProxyEditSource_Impl::UpdateData()
    {
        maEditSource.UpdateData();
    }

    SfxBroadcaster& AccessibleProxyEditSource_Impl::GetBroadcaster() const
    {
        return maEditSource.GetBroadcaster();
    }


    // -------------------------------------------------------------------------
    // Implementing AccessibleEmptyEditSource
    // -------------------------------------------------------------------------

    AccessibleEmptyEditSource::AccessibleEmptyEditSource( SdrObject&    rObj,
                                                          SdrView&      rView,
                                                          const Window& rViewWindow ) :
        mpEditSource( new AccessibleEmptyEditSource_Impl() ),
        mrObj(rObj),
        mrView(rView),
        mrViewWindow(rViewWindow),
        mbEditSourceEmpty( true )
    {
        StartListening( mrObj.getSdrModelFromSdrObject() );
    }

    AccessibleEmptyEditSource::~AccessibleEmptyEditSource()
    {
        if( !mbEditSourceEmpty )
        {
            // deregister as listener
            if( mpEditSource.get() )
                EndListening( mpEditSource->GetBroadcaster() );
        }
        else
        {
            EndListening( mrObj.getSdrModelFromSdrObject() );
        }
    }

    SvxTextForwarder* AccessibleEmptyEditSource::GetTextForwarder()
    {
        if( !mpEditSource.get() )
            return NULL;

        return mpEditSource->GetTextForwarder();
    }

    SvxViewForwarder* AccessibleEmptyEditSource::GetViewForwarder()
    {
        if( !mpEditSource.get() )
            return NULL;

        return mpEditSource->GetViewForwarder();
    }

    void AccessibleEmptyEditSource::Switch2ProxyEditSource()
    {
        // deregister EmptyEditSource model listener
        EndListening( mrObj.getSdrModelFromSdrObject() );

        ::std::auto_ptr< SvxEditSource > pProxySource( new AccessibleProxyEditSource_Impl(mrObj, mrView, mrViewWindow) );
        ::std::auto_ptr< SvxEditSource > tmp = mpEditSource;
        mpEditSource = pProxySource;
        pProxySource = tmp;

        // register as listener
        StartListening( mpEditSource->GetBroadcaster() );

        // we've irrevocably a full EditSource now.
        mbEditSourceEmpty = false;
    }

    SvxEditViewForwarder* AccessibleEmptyEditSource::GetEditViewForwarder( sal_Bool bCreate )
    {
        if( !mpEditSource.get() )
            return NULL;

        // switch edit source, if not yet done
        if( mbEditSourceEmpty && bCreate )
            Switch2ProxyEditSource();

        return mpEditSource->GetEditViewForwarder( bCreate );
    }

    SvxEditSource* AccessibleEmptyEditSource::Clone() const
    {
        if( !mpEditSource.get() )
            return NULL;

        return mpEditSource->Clone();
    }

    void AccessibleEmptyEditSource::UpdateData()
    {
        if( mpEditSource.get() )
            mpEditSource->UpdateData();
    }

    SfxBroadcaster& AccessibleEmptyEditSource::GetBroadcaster() const
    {
        return *(const_cast<AccessibleEmptyEditSource*>(this));
    }

    void AccessibleEmptyEditSource::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
    {
        const SdrBaseHint* pSdrHint = dynamic_cast< const SdrBaseHint* >(&rHint);

        if( pSdrHint && pSdrHint->GetSdrHintKind() == HINT_BEGEDIT &&
            &mrObj == pSdrHint->GetSdrHintObject() && mpEditSource.get() )
        {
            // switch edit source, if not yet done. This is necessary
            // to become a full-fledged EditSource the first time a
            // user start entering text in a previously empty object.
            if( mbEditSourceEmpty )
                Switch2ProxyEditSource();
        }
        else if (pSdrHint && pSdrHint->GetSdrHintObject()!=NULL)
        {
            // When the SdrObject just got a para outliner object then
            // switch the edit source.
            if (pSdrHint->GetSdrHintObject()->GetOutlinerParaObject() != NULL)
                Switch2ProxyEditSource();
        }

        // forward messages
        Broadcast( rHint );
    }

} // end of namespace accessibility

//------------------------------------------------------------------------
