/*************************************************************************
 *
 *  $RCSfile: rtattributehandler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-07 16:15:01 $
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

#ifndef FORMS_SOURCE_COMPONENT_RTATTRIBUTEHANDLER_HXX
#define FORMS_SOURCE_COMPONENT_RTATTRIBUTEHANDLER_HXX

#ifndef FORMS_SOURCE_RICHTEXT_RTATTRIBUTES_HXX
#include "rtattributes.hxx"
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _SVX_SVXENUM_HXX
#include <svx/svxenum.hxx>
#endif
#ifndef _SVX_FRMDIR_HXX
#include <svx/frmdir.hxx>
#endif

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

