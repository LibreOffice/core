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

#ifndef ADC_X_PARSE_HXX
#define ADC_X_PARSE_HXX

// BASE CLASSES
#include <autodoc/x_parsing.hxx>




class X_Parser : public autodoc::X_Parser_Ifc
{
  public:
    // LIFECYCLE
                        X_Parser(
                            E_Event				i_eEvent,
                            const char *		i_sObject,
                            const String &      i_sCausingFile_FullPath,
                            uintt				i_nCausingLineNr );
                        ~X_Parser();
    // INQUIRY
    virtual E_Event	   	GetEvent() const;
    virtual void	   	GetInfo(
                            std::ostream &      o_rOutputMedium ) const;

  private:
    E_Event				eEvent;
    String 				sObject;
    String 				sCausingFile_FullPath;
    uintt				nCausingLineNr;

};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
