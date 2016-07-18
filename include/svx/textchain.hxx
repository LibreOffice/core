/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_TEXTCHAIN_HXX
#define INCLUDED_SVX_TEXTCHAIN_HXX

#include <editeng/editdata.hxx>
#include <map>

/*
 * Properties can be accessed and set from a TextChain with:
 * - T TextChain::GetPROPNAME(SdrTextObj *)
 * - void TextChain::SetPROPNAME(SdrTextObj *, T)
 * where T and PROPNAME are respectively type and name of a property.
 *
 * To add a property PROPNAME of type T (and its interface) in TextChain:
 * 1) Add
 *      "DECL_CHAIN_PROP(PROPNAME, T)"
 *    in class ImpChainLinkProperties;
 * 2) Add
 *      "INIT_CHAIN_PROP(PROPNAME, V)"
 *    in constructor of ImpChainLinkProperties below
 *    (V is the initialization value for PROPNAME)
 *
 * 3) Add
 *      "DECL_CHAIN_PROP_INTERFACE(PROPNAME, T)"
 *    in class TextChain (under "public:");
 * 4)  Add
 *       "IMPL_CHAIN_PROP_INTERFACE(PROPNAME, T)"
 *    in file "svx/source/svdraw/textchain.cxx"
*/

#define DECL_CHAIN_PROP(PropName, PropType) \
    PropType a##PropName;

#define INIT_CHAIN_PROP(PropName, PropDefault) \
    a##PropName = (PropDefault);

#define DECL_CHAIN_PROP_INTERFACE(PropName, PropType) \
    PropType const & Get##PropName (const SdrTextObj *); \
    void Set##PropName (const SdrTextObj *, PropType);

#define IMPL_CHAIN_PROP_INTERFACE(PropName, PropType) \
    PropType const & TextChain::Get##PropName (const SdrTextObj *pTarget) { \
        ImpChainLinkProperties *pLinkProperties = GetLinkProperties(pTarget); \
        return pLinkProperties->a##PropName; \
    } \
    void TextChain::Set##PropName (const SdrTextObj *pTarget, PropType aPropParam) \
    { \
        ImpChainLinkProperties *pLinkProperties = GetLinkProperties(pTarget); \
        pLinkProperties->a##PropName = aPropParam; \
    }

/* End Special Properties Macro */


class ImpChainLinkProperties;
class SdrTextObj;
class SdrModel;

namespace rtl {
    class OUString;
}

typedef rtl::OUString ChainLinkId;

enum class CursorChainingEvent
{
    TO_NEXT_LINK,
    TO_PREV_LINK,
    UNCHANGED,
    NULL_EVENT
};

class ImpChainLinkProperties
{
protected:
    friend class TextChain;

    ImpChainLinkProperties() {
        INIT_CHAIN_PROP(NilChainingEvent, false)
        INIT_CHAIN_PROP(CursorEvent, CursorChainingEvent::NULL_EVENT)
        INIT_CHAIN_PROP(PreChainingSel, ESelection(0,0,0,0));
        INIT_CHAIN_PROP(PostChainingSel, ESelection(0,0,0,0));
        INIT_CHAIN_PROP(IsPartOfLastParaInNextLink, false) // XXX: Should come from file
        INIT_CHAIN_PROP(PendingOverflowCheck, false)
        INIT_CHAIN_PROP(SwitchingToNextBox, false)
    }

private:
    // NOTE: Remember to set default value in constructor when adding field
    DECL_CHAIN_PROP(NilChainingEvent, bool)
    DECL_CHAIN_PROP(CursorEvent, CursorChainingEvent)
    DECL_CHAIN_PROP(PreChainingSel, ESelection)
    DECL_CHAIN_PROP(PostChainingSel, ESelection)
    DECL_CHAIN_PROP(IsPartOfLastParaInNextLink, bool)
    DECL_CHAIN_PROP(PendingOverflowCheck, bool)
    DECL_CHAIN_PROP(SwitchingToNextBox, bool)
};


class TextChain
{
public:
    ~TextChain();

    //void AppendLink(SdrTextObj *);
    //bool IsLinkInChain(SdrTextObj *) const;

    //SdrTextObj *GetNextLink(const SdrTextObj *) const;
    //SdrTextObj *GetPrevLink(const SdrTextObj *) const;

    ImpChainLinkProperties *GetLinkProperties(const SdrTextObj *);

    // Specific Link Properties
    DECL_CHAIN_PROP_INTERFACE(CursorEvent, CursorChainingEvent)
    DECL_CHAIN_PROP_INTERFACE(NilChainingEvent, bool)
    DECL_CHAIN_PROP_INTERFACE(PreChainingSel, ESelection)
    DECL_CHAIN_PROP_INTERFACE(PostChainingSel, ESelection)
    // return whether a paragraph is split between this box and the next
    DECL_CHAIN_PROP_INTERFACE(IsPartOfLastParaInNextLink, bool)
    // return whether there is a pending overflow check (usually when we move cursor after an overflow in the prev link)
    DECL_CHAIN_PROP_INTERFACE(PendingOverflowCheck, bool)
    // return whether we are currently moving the cursor to the next box (useful to know whether we should prevent SetOutlinerParaObject invocations in SdrTextObj::EndTextEdit)
    DECL_CHAIN_PROP_INTERFACE(SwitchingToNextBox, bool)

protected:
    TextChain();
    std::map< ChainLinkId, ImpChainLinkProperties *> maLinkPropertiesMap;

private:
    friend class SdrModel;
    //SdrTextObj *impGetNextLink(const SdrTextObj *) const;
    //SdrTextObj *impGetPrevLink(const SdrTextObj *) const;
};

#endif // INCLUDED_SVX_TEXTCHAIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
