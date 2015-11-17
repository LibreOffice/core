/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_OOX_XLS_REVISIONFRAGMENT_HXX
#define INCLUDED_SC_OOX_XLS_REVISIONFRAGMENT_HXX

#include <excelhandlers.hxx>
#include <memory>

class ScChangeTrack;

namespace oox { namespace xls {

class RevisionHeadersFragment : public WorkbookFragmentBase
{
    struct Impl;
    std::unique_ptr<Impl> mpImpl;

public:
    explicit RevisionHeadersFragment(
        const WorkbookHelper& rHelper, const OUString& rFragmentPath );

    virtual ~RevisionHeadersFragment();

protected:
    virtual oox::core::ContextHandlerRef onCreateContext(
        sal_Int32 nElement, const AttributeList& rAttribs ) override;

    virtual void onStartElement( const AttributeList& rAttribs ) override;
    virtual void onCharacters( const OUString& rChars ) override;
    virtual void onEndElement() override;

    virtual void finalizeImport() override;

private:
    void importHeader( const AttributeList& rAttribs );
};

class RevisionLogFragment : public WorkbookFragmentBase
{
    struct Impl;
    std::unique_ptr<Impl> mpImpl;

public:
    explicit RevisionLogFragment(
        const WorkbookHelper& rHelper, const OUString& rFragmentPath, ScChangeTrack& rChangeTrack );

    virtual ~RevisionLogFragment();

protected:
    virtual oox::core::ContextHandlerRef onCreateContext(
        sal_Int32 nElement, const AttributeList& rAttribs ) override;

    virtual void onStartElement( const AttributeList& rAttribs ) override;
    virtual void onCharacters( const OUString& rChars ) override;
    virtual void onEndElement() override;

    virtual void finalizeImport() override;

private:
    void importCommon( const AttributeList& rAttribs );
    void importRcc( const AttributeList& rAttribs );
    void importRrc( const AttributeList& rAttribs );

    void pushRevision();
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

