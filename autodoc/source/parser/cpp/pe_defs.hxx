/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_defs.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:53:37 $
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

#ifndef ADC_CPP_PE_DEFS_HXX
#define ADC_CPP_PE_DEFS_HXX




// USED SERVICES
    // BASE CLASSES
#include "cpp_pe.hxx"
    // COMPONENTS
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
    // PARAMETERS


namespace cpp
{



class PE_Defines : public cpp::Cpp_PE
{
  public:
    enum E_State
    {
        expectName,
        gotDefineName,
        expectMacroParameters,
        size_of_states
    };

                        PE_Defines(
                            Cpp_PE *            i_pParent );
                        ~PE_Defines();

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );

  private:
    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError( const char *);

    void                On_expectName_DefineName( const char * );
    void                On_expectName_MacroName( const char * );

    void                On_gotDefineName_PreProDefinition( const char * );

    void                On_expectMacroParameters_MacroParameter( const char * );
    void                On_expectMacroParameters_PreProDefinition( const char * );

    // DATA
    Dyn< PeStatusArray<PE_Defines> >
                        pStati;

    String              sName;
    StringVector        aParameters;
    StringVector        aDefinition;
    bool                bIsMacro;
};



}   //namespace cpp
#endif

