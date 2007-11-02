/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: x_parse.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:02:42 $
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

#ifndef ADC_X_PARSE_HXX
#define ADC_X_PARSE_HXX

// BASE CLASSES
#include <autodoc/x_parsing.hxx>




class X_Parser : public autodoc::X_Parser_Ifc
{
  public:
    // LIFECYCLE
                        X_Parser(
                            E_Event             i_eEvent,
                            const char *        i_sObject,
                            const String &      i_sCausingFile_FullPath,
                            uintt               i_nCausingLineNr );
                        ~X_Parser();
    // INQUIRY
    virtual E_Event     GetEvent() const;
    virtual void        GetInfo(
                            std::ostream &      o_rOutputMedium ) const;

  private:
    E_Event             eEvent;
    String              sObject;
    String              sCausingFile_FullPath;
    uintt               nCausingLineNr;

};




#endif
