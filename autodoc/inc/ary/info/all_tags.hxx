/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: all_tags.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:14:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

