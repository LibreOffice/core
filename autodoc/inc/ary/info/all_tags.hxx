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

#ifndef ARY_INFO_ALL_TAGS_HXX
#define ARY_INFO_ALL_TAGS_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/info/ci_attag.hxx>
    // COMPONENTS
#include <ary/info/inftypes.hxx>
#include <ary/info/ci_text.hxx>
#include <ary/qualiname.hxx>
    // PARAMETERS



namespace ary
{
namespace info
{


class StdTag : public AtTag
{
  public:
                        StdTag(
                            E_AtTagId       i_eId );

    virtual bool        Add_SpecialMeaningToken(
                            const char *        i_sText,
                            intt                i_nNr );
    void                ChangeId2(
                            E_AtTagId           i_eId )
                                                { eId = i_eId; }

    virtual UINT8       NrOfSpecialMeaningTokens() const;
    virtual AtTag *     GetFollower();

    E_AtTagId           Std_Id() const          { return eId; }

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual DocuText *  Text();

    E_AtTagId           eId;
    DocuText            aText;
    StdTag *            pNext;
};

class BaseTag : public AtTag
{
  public:
                        BaseTag();

    virtual bool        Add_SpecialMeaningToken(
                            const char *        i_sText,
                            intt                i_nNr );
    virtual const char *
                        Title() const;
    virtual UINT8       NrOfSpecialMeaningTokens() const;
    virtual AtTag *     GetFollower();

  private:
    virtual DocuText *  Text();

    QualifiedName       sBase;
    DocuText            aText;
    AtTag *             pNext;
};

class ExceptionTag : public AtTag
{
  public:
                        ExceptionTag();

    virtual bool        Add_SpecialMeaningToken(
                            const char *        i_sText,
                            intt                i_nNr );
    virtual const char *
                        Title() const;
    virtual UINT8       NrOfSpecialMeaningTokens() const;
    virtual AtTag *     GetFollower();

  private:
    virtual DocuText *  Text();

    QualifiedName       sException;
    DocuText            aText;
    AtTag *             pNext;
};

class ImplementsTag : public AtTag
{
  public:
                        ImplementsTag();

    virtual bool        Add_SpecialMeaningToken(
                            const char *        i_sText,
                            intt                i_nNr );
    virtual const char *
                        Title() const;
    virtual UINT8       NrOfSpecialMeaningTokens() const;
    virtual AtTag *     GetFollower();

  private:
    virtual DocuText *  Text();

    QualifiedName   sName;
    AtTag *         pNext;
};

class ParameterTag : public AtTag
{
  public:
                        ParameterTag();

    virtual bool        Add_SpecialMeaningToken(
                            const char *        i_sText,
                            intt                i_nNr );

    const String  &     ParamName() const       { return sName; }
    const DocuText &    CText() const           { return aText; }
    virtual UINT8       NrOfSpecialMeaningTokens() const;
    virtual const ParameterTag *
                        GetNext() const         { return dynamic_cast< ParameterTag* >(pNext); }
    virtual AtTag *     GetFollower();

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual DocuText *  Text();

    String              sName;
    String              sValidRange;
    DocuText            aText;
    AtTag *             pNext;
};

class TemplateTag : public AtTag
{
  public:
                        TemplateTag();

    virtual bool        Add_SpecialMeaningToken(
                            const char *        i_sText,
                            intt                i_nNr );
    const DocuText &    CText() const           { return aText; }
    virtual const char *
                        Title() const;
    const String  &     TplParamName() const    { return sName; }
    virtual UINT8       NrOfSpecialMeaningTokens() const;
    virtual const TemplateTag *
                        GetNext() const         { return dynamic_cast< TemplateTag* >(pNext); }
    virtual AtTag *     GetFollower();

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual DocuText *  Text();

    String              sName;
    DocuText            aText;
    AtTag *             pNext;
};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
