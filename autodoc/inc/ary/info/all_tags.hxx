/*************************************************************************
 *
 *  $RCSfile: all_tags.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:11:19 $
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
#include <ary/ids.hxx>



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

class KeywordTag : public AtTag
{
  public:
                        KeywordTag();

    virtual bool        Add_SpecialMeaningToken(
                            const char *        i_sText,
                            intt                i_nNr );
    virtual const char *
                        Title() const;
    virtual UINT8       NrOfSpecialMeaningTokens() const;
    virtual AtTag *     GetFollower();

  private:
    virtual DocuText *  Text();

    StringVector        sKeys;
};

class ParameterTag : public AtTag
{
  public:
                        ParameterTag();

    virtual bool        Add_SpecialMeaningToken(
                            const char *        i_sText,
                            intt                i_nNr );

    const udmstri &     ParamName() const       { return sName; }
    const DocuText &    CText() const           { return aText; }
    virtual UINT8       NrOfSpecialMeaningTokens() const;
    virtual const ParameterTag *
                        GetNext() const         { return dynamic_cast< ParameterTag* >(pNext); }
    virtual AtTag *     GetFollower();

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual DocuText *  Text();

    udmstri             sName;
    udmstri             sValidRange;
    DocuText            aText;
    AtTag *             pNext;
};

class SeeTag : public AtTag
{
  public:
                        SeeTag();

    virtual bool        Add_SpecialMeaningToken(
                            const char *        i_sText,
                            intt                i_nNr );
    virtual const char *
                        Title() const;
    virtual UINT8       NrOfSpecialMeaningTokens() const;
    virtual AtTag *     GetFollower();

    const std::vector< QualifiedName > &
                        References() const      { return sReferences; }
  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual DocuText *  Text();

    std::vector< QualifiedName >
                        sReferences;
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
    const udmstri &     TplParamName() const    { return sName; }
    virtual UINT8       NrOfSpecialMeaningTokens() const;
    virtual const TemplateTag *
                        GetNext() const         { return dynamic_cast< TemplateTag* >(pNext); }
    virtual AtTag *     GetFollower();

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual DocuText *  Text();

    udmstri             sName;
    DocuText            aText;
    AtTag *             pNext;
};

class LabelTag : public AtTag
{
  public:
                        LabelTag();

    virtual bool        Add_SpecialMeaningToken(
                            const char *        i_sText,
                            intt                i_nNr );
    virtual const char *
                        Title() const;
    virtual UINT8       NrOfSpecialMeaningTokens() const;
    virtual AtTag *     GetFollower();

  private:
    virtual DocuText *  Text();

    String              sLabel;
};

class SinceTag : public AtTag
{
  public:
                        SinceTag();

    virtual bool        Add_SpecialMeaningToken(
                            const char *        i_sText,
                            intt                i_nNr );
    virtual const char *
                        Title() const;
    virtual UINT8       NrOfSpecialMeaningTokens() const;
    virtual AtTag *     GetFollower();

    const String &      Version() const         { return sVersion; }

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual DocuText *  Text();

    // Data
    String              sVersion;
};



// IMPLEMENTATION


}
}

#endif

