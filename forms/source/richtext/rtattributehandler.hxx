/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rtattributehandler.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef FORMS_SOURCE_COMPONENT_RTATTRIBUTEHANDLER_HXX
#define FORMS_SOURCE_COMPONENT_RTATTRIBUTEHANDLER_HXX

#include "rtattributes.hxx"
#include <rtl/ref.hxx>
#include <svx/svxenum.hxx>
#include <svx/frmdir.hxx>

class SfxItemSet;
class SfxPoolItem;
class SfxItemPool;
//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= ReferenceBase
    //====================================================================
    class ReferenceBase : public ::rtl::IReference
    {
    protected:
        oslInterlockedCount m_refCount;

    public:
        // IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

    protected:
        virtual ~ReferenceBase();
    };

    //====================================================================
    //= IAttributeHandler
    //====================================================================
    class IAttributeHandler : public ::rtl::IReference
    {
    public:
        virtual     AttributeId     getAttributeId( ) const = 0;
        virtual     AttributeState  getState( const SfxItemSet& _rAttribs ) const = 0;
        virtual     void            executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const = 0;
    };

    //====================================================================
    //= AttributeHandler
    //====================================================================
    class AttributeHandler  :public ReferenceBase
                            ,public IAttributeHandler
    {
    private:
        AttributeId     m_nAttribute;
        WhichId         m_nWhich;

    protected:
        AttributeId getAttribute() const { return m_nAttribute; }
        WhichId     getWhich() const     { return m_nWhich;     }

    public:
        AttributeHandler( AttributeId _nAttributeId, WhichId _nWhichId );

        // IAttributeHandler
        virtual     AttributeId     getAttributeId( ) const;
        virtual     AttributeState  getState( const SfxItemSet& _rAttribs ) const;
        virtual     void            executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const = 0;

    protected:
        /// helper method calling implGetCheckState
        AttributeCheckState getCheckState( const SfxItemSet& _rAttribs ) const;

        /// helper method putting an item into a set, respecting a script type
        void        putItemForScript( SfxItemSet& _rAttribs, const SfxPoolItem& _rItem, ScriptType _nForScriptType ) const;

        // pseudo-abstract
        virtual     AttributeCheckState  implGetCheckState( const SfxPoolItem& _rItem ) const;

        // disambiguate IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

    protected:
        virtual ~AttributeHandler();
    };

    //====================================================================
    //= AttributeHandlerFactory
    //====================================================================
    class AttributeHandlerFactory
    {
    public:
        static ::rtl::Reference< IAttributeHandler > getHandlerFor( AttributeId _nAttributeId, const SfxItemPool& _rEditEnginePool );

    private:
        AttributeHandlerFactory();                                              // never implemented
        AttributeHandlerFactory( const AttributeHandlerFactory& );              // never implemented
        AttributeHandlerFactory& operator=( const AttributeHandlerFactory& );   // never implemented
        ~AttributeHandlerFactory();                                             // never implemented
    };

    //====================================================================
    //= ParaAlignmentHandler
    //====================================================================
    class ParaAlignmentHandler : public AttributeHandler
    {
    private:
        SvxAdjust   m_eAdjust;

    public:
        ParaAlignmentHandler( AttributeId _nAttributeId );

    public:
        virtual     AttributeCheckState implGetCheckState( const SfxPoolItem& _rItem ) const;
        virtual     void                executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const;
    };

    //====================================================================
    //= LineSpacingHandler
    //====================================================================
    class LineSpacingHandler : public AttributeHandler
    {
    private:
        USHORT  m_nLineSpace;

    public:
        LineSpacingHandler( AttributeId _nAttributeId );

    public:
        virtual     AttributeCheckState implGetCheckState( const SfxPoolItem& _rItem ) const;
        virtual     void                executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const;
    };

    //====================================================================
    //= EscapementHandler
    //====================================================================
    class EscapementHandler : public AttributeHandler
    {
    private:
        SvxEscapement   m_eEscapement;

    public:
        EscapementHandler( AttributeId _nAttributeId );

    public:
        virtual     AttributeCheckState implGetCheckState( const SfxPoolItem& _rItem ) const;
        virtual     void                executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const;
    };

    //====================================================================
    //= SlotHandler
    //====================================================================
    class SlotHandler : public AttributeHandler
    {
    private:
        bool    m_bScriptDependent;

    public:
        SlotHandler( AttributeId _nAttributeId, WhichId _nWhichId );

    public:
        virtual     AttributeState  getState( const SfxItemSet& _rAttribs ) const;
        virtual     void            executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const;
    };

    //====================================================================
    //= BooleanHandler
    //====================================================================
    class BooleanHandler : public AttributeHandler
    {
    public:
        BooleanHandler( AttributeId _nAttributeId, WhichId _nWhichId );

    public:
        virtual     AttributeCheckState implGetCheckState( const SfxPoolItem& _rItem ) const;
        virtual     void                executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const;
    };

    //====================================================================
    //= FontSizeHandler
    //====================================================================
    class FontSizeHandler : public AttributeHandler
    {
    public:
        FontSizeHandler( AttributeId _nAttributeId, WhichId _nWhichId );

    public:
        virtual     AttributeState  getState( const SfxItemSet& _rAttribs ) const;
        virtual     void            executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const;
    };

    //====================================================================
    //= ParagraphDirectionHandler
    //====================================================================
    class ParagraphDirectionHandler : public AttributeHandler
    {
    private:
        SvxFrameDirection   m_eParagraphDirection;
        SvxAdjust           m_eDefaultAdjustment;
        SvxAdjust           m_eOppositeDefaultAdjustment;

    public:
        ParagraphDirectionHandler( AttributeId _nAttributeId );

    public:
        virtual     AttributeCheckState implGetCheckState( const SfxPoolItem& _rItem ) const;
        virtual     void                executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const;
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_COMPONENT_RTATTRIBUTEHANDLER_HXX

