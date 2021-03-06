/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "excelhandlers.hxx"
#include <oox/core/contexthandler.hxx>
#include "condformatbuffer.hxx"

#include <vector>
#include <memory>

extern sal_Int32 rStyleIdx; // Holds index of the <extlst> <cfRule> style (Will be reset by finalize import)

struct ScDataBarFormatData;
namespace oox { class AttributeList; }
namespace oox::xls { class WorkbookFragment; }
namespace oox::xls { class WorksheetFragment; }

namespace oox::xls {

class ExtCfRuleContext : public WorksheetContextBase
{
public:
    explicit ExtCfRuleContext( WorksheetContextBase& rFragment, ScDataBarFormatData* pDataBar );

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onStartElement( const AttributeList& rAttribs ) override;

private:
    ScDataBarFormatData* mpTarget;

    bool mbFirstEntry;
};

struct ExtCondFormatRuleModel
{
    sal_Int32 nPriority;
    ScConditionMode eOperator;
    OUString aFormula;
    OUString aStyle;
};

class ExtConditionalFormattingContext : public WorksheetContextBase
{
public:
    explicit ExtConditionalFormattingContext(WorksheetContextBase& rFragment);

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void onStartElement( const AttributeList& rAttribs ) override;
    virtual void onCharacters(const OUString& rCharacters) override;
    virtual void onEndElement() override;

private:
    ExtCondFormatRuleModel maModel;
    sal_Int32 nFormulaCount;
    OUString aChars; // Characters of between xml elements.
    sal_Int32 nPriority; // Priority of last cfRule element.
    ScConditionMode eOperator; // Used only when cfRule type is "cellIs"
    bool isPreviousElementF;   // Used to distinguish alone <sqref> from <f> and <sqref>
    std::vector<std::unique_ptr<ScFormatEntry> > maEntries;
    std::unique_ptr<IconSetRule> mpCurrentRule;
    std::vector<sal_Int32> maPriorities;
    std::vector<ExtCondFormatRuleModel> maModels;
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
    explicit ExtLstLocalContext( WorksheetContextBase& rFragment, ScDataBarFormatData* pTarget ); // until now a ExtLst always extends an existing entry

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onStartElement( const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;

private:
    ScDataBarFormatData* mpTarget;
};

/**
 * A single ext entry. Will be skipped until the actual entry with the correct uri is found
 */
class ExtGlobalContext : public WorksheetContextBase
{
public:
    explicit ExtGlobalContext( WorksheetContextBase& rFragment );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onStartElement( const AttributeList& rAttribs ) override;

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
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};

class ExtGlobalWorkbookContext : public WorkbookContextBase
{
public:
    explicit ExtGlobalWorkbookContext( WorkbookContextBase& rFragment );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onStartElement( const AttributeList& rAttribs ) override;

private:
};

class ExtLstGlobalWorkbookContext : public WorkbookContextBase
{
public:
    explicit ExtLstGlobalWorkbookContext( WorkbookFragment& rFragment );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};

} //namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
