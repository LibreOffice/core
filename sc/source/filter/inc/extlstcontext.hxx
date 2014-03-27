/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_EXTLSTCONTEXT_HXX
#define SC_EXTLSTCONTEXT_HXX

#include "excelhandlers.hxx"
#include "worksheetfragment.hxx"

namespace oox {
namespace xls {

class ExtCfRuleContext : public WorksheetContextBase
{
public:
    explicit ExtCfRuleContext( WorksheetContextBase& rFragment, void* pDataBar );

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) SAL_OVERRIDE;
    virtual void        onStartElement( const AttributeList& rAttribs ) SAL_OVERRIDE;

private:
    void* mpTarget;

    bool mbFirstEntry;
};

/**
 * Handle ExtLst entries in xlsx. These entries are a way to extend the standard
 * without actually changing it
 *
 * Needed right now for data bars
 *
 * ExtLstLocalContext is for the entry in the datastructure that needs to be extended
 */
class ExtLstLocalContext : public WorksheetContextBase
{
public:
    explicit ExtLstLocalContext( WorksheetContextBase& rFragment, void* pTarget ); // until now a ExtLst always extends an existing entry

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) SAL_OVERRIDE;
    virtual void        onStartElement( const AttributeList& rAttribs ) SAL_OVERRIDE;
    virtual void        onCharacters( const OUString& rChars ) SAL_OVERRIDE;

private:
    void* mpTarget;
};

/**
 * A single ext entry. Will be skipped until the actual entry with the correct uri is found
 */
class ExtGlobalContext : public WorksheetContextBase
{
public:
    explicit ExtGlobalContext( WorksheetContextBase& rFragment );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) SAL_OVERRIDE;
    virtual void        onStartElement( const AttributeList& rAttribs ) SAL_OVERRIDE;

private:
};

/**
 * Used for the actual ExtLst containing the new extended definition.
 * Uses the saved data from the ExtLstLocalContext
 */
class ExtLstGlobalContext : public WorksheetContextBase
{
public:
    explicit ExtLstGlobalContext( WorksheetFragment& rFragment );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) SAL_OVERRIDE;
};

} //namespace xls
} //namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
