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
 * 2) Add
 *      "DECL_CHAIN_PROP_INTERFACE(PROPNAME, T)"
 *    in class TextChain (under "public:");
 * 3)  Add
 *       "IMPL_CHAIN_PROP_INTERFACE(PROPNAME, T)"
 *    in file "svx/source/svdraw/textchain.cxx"
*/

#define DECL_CHAIN_PROP(PropName, PropType) \
    PropType a##PropName;

#define INIT_CHAIN_PROP(PropName, PropDefault) \
    a##PropName = (PropDefault);

#define DECL_CHAIN_PROP_INTERFACE(PropName, PropType) \
    PropType Get##PropName (const SdrTextObj *); \
    void Set##PropName (const SdrTextObj *, PropType);

#define IMPL_CHAIN_PROP_INTERFACE(PropName, PropType) \
    PropType TextChain::Get##PropName (const SdrTextObj *pTarget) { \
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

namespace rtl {
    class OUString;
}

typedef rtl::OUString ChainLinkId;
typedef std::map< ChainLinkId, ImpChainLinkProperties *> LinkPropertiesMap;

enum class CursorChainingEvent
{
    TO_NEXT_LINK,
    TO_PREV_LINK,
    UNCHANGED,
    NULL_EVENT
};

/*
 * // Note Sure I need the code here
struct CursorChainingInfo
{
    public:
        CursorChainingEvent GetEvent() const;
        ESelection GetPreChainingSel() const;
        ESelection GetPostChainingSel() const;

        void SetEvent(CursorChainingEvent aEvent) { maEvent = aEvent; }
        void SetPreChainingSel(ESelection aSel) { maPreChainingSel = aSel; }
        void SetPostChainingSel(ESelection aSel) { maPostChaingingSel = aSel; }
    private:
        CursorChainingEvent maEvent;
        ESelection maPreChainingSel;
        ESelection maPostChainingSel;
};
* */

class ImpChainLinkProperties
{
    protected:
    friend class TextChain;

    ImpChainLinkProperties() {
        INIT_CHAIN_PROP(NilChainingEvent, false)
        INIT_CHAIN_PROP(CursorEvent, CursorChainingEvent::NULL_EVENT)
        INIT_CHAIN_PROP(PreChainingSel, ESelection(0,0,0,0));
        INIT_CHAIN_PROP(PostChainingSel, ESelection(0,0,0,0));
        INIT_CHAIN_PROP(IsPartOfLastParaInNextLink, false)
    }

    private:
    // NOTE: Remember to set default value in contructor when adding field
    DECL_CHAIN_PROP(NilChainingEvent, bool)
    DECL_CHAIN_PROP(CursorEvent, CursorChainingEvent)
    DECL_CHAIN_PROP(PreChainingSel, ESelection)
    DECL_CHAIN_PROP(PostChainingSel, ESelection)
    DECL_CHAIN_PROP(IsPartOfLastParaInNextLink, bool)

};


class TextChain {

    public:
    TextChain();
    ~TextChain();

    void AppendLink(SdrTextObj *);
    bool IsLinkInChain(SdrTextObj *) const;
    SdrTextObj *GetNextLink(SdrTextObj *) const;

    ChainLinkId GetId(const SdrTextObj *) const;
    ImpChainLinkProperties *GetLinkProperties(const SdrTextObj *);

    // Specific Link Properties
    DECL_CHAIN_PROP_INTERFACE(CursorEvent, CursorChainingEvent)
    DECL_CHAIN_PROP_INTERFACE(NilChainingEvent, bool)
    DECL_CHAIN_PROP_INTERFACE(PreChainingSel, ESelection)
    DECL_CHAIN_PROP_INTERFACE(PostChainingSel, ESelection)
    // return whether a paragraph is split between this box and the next
    DECL_CHAIN_PROP_INTERFACE(IsPartOfLastParaInNextLink, bool)



    protected:
    LinkPropertiesMap maLinkPropertiesMap;
};

#endif // INCLUDED_SVX_TEXTCHAIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

