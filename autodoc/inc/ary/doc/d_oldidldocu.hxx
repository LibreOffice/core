/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
