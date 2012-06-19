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

#ifndef ARY_DOC_D_OLDIDLDOCU_HXX
#define ARY_DOC_D_OLDIDLDOCU_HXX

// BASE CLASSES
#include <ary/doc/d_node.hxx>
// USED SERVICES
#include <ary_i/ci_text2.hxx>




namespace ary
{
namespace inf
{
    class AtTag2;
    class DocuToken;
    class DocuTex2;
}
namespace doc
{
    using ::ary::inf::AtTag2;
    using ::ary::inf::DocuToken;
    using ::ary::inf::DocuTex2;



/** Wrapper for the old idl documentation format.
*/
class OldIdlDocu : public Node
{
  public:
                        OldIdlDocu();
                        ~OldIdlDocu();

    void                AddToken2Short(
                            DYN DocuToken &     let_drToken )
                                                { aShort.AddToken(let_drToken); }
    void                AddToken2Description(
                            DYN DocuToken &     let_drToken )
                                                { aDescription.AddToken(let_drToken); }
    void                AddToken2DeprecatedText(
                            DYN DocuToken &     let_drToken );
    void                AddAtTag(
                            DYN AtTag2 &        let_drAtTag )
                                                { aTags.push_back(&let_drAtTag); }
    void                SetPublished()          { bIsPublished = true; }
    void                SetDeprecated()         { bIsDeprecated = true; }
    void                SetOptional()           { bIsOptional = true; }
    void                SetExternShort(
                            const DocuTex2 &    i_pExternShort )
                                                { pExternShort = &i_pExternShort; }

    const DocuTex2 &    Short() const           { return pExternShort != 0 ? *pExternShort : aShort; }
    const DocuTex2 &    Description() const     { return aDescription; }
    const DocuTex2 &    DeprecatedText() const  { return aDeprecatedText; }
    const std::vector< AtTag2* > &
                        Tags() const            { return aTags; }
    bool                IsPublished() const     { return bIsPublished; }
    bool                IsDeprecated() const    { return bIsDeprecated; }
    bool                IsOptional() const      { return bIsOptional; }

  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // DATA
    DocuTex2            aShort;
    DocuTex2            aDescription;
    DocuTex2            aDeprecatedText;
    std::vector< AtTag2* >
                        aTags;
    const DocuTex2 *    pExternShort;
    bool                bIsPublished;
    bool                bIsDeprecated;
    bool                bIsOptional;
};




}   // namespace doc
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
