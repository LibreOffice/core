/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tk_docw.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:03:14 $
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

#ifndef ADC_ADOC_TK_DOCW_HXX
#define ADC_ADOC_TK_DOCW_HXX

// USED SERVICES
    // BASE CLASSES
#include <adoc/adoc_tok.hxx>
    // COMPONENTS
    // PARAMETERS

namespace adoc {


class Tok_DocWord : public Token
{
  public:
    // Spring and Fall
                        Tok_DocWord(
                            const char *        i_sText )
                                                :   sText(i_sText) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    uintt               Length() const          { return sText.length(); }

  private:
    // DATA
    String              sText;
};

class Tok_Whitespace : public Token
{
  public:
    // Spring and Fall
                        Tok_Whitespace(
                            UINT8               i_nSize )
                                                :   nSize(i_nSize) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    UINT8               Size() const            { return nSize; }

  private:
    // DATA
    UINT8               nSize;
};

class Tok_LineStart : public Token
{
  public:
    // Spring and Fall
                        Tok_LineStart(
                            UINT8               i_nSize )
                                                :   nSize(i_nSize) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    UINT8               Size() const            { return nSize; }

  private:
    // DATA
    UINT8               nSize;
};

class Tok_Eol : public Token
{ public:
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual const char *
                        Text() const;
};

class Tok_EoDocu : public Token
{ public:
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual const char *
                        Text() const;
};


}   // namespace adoc

#endif

