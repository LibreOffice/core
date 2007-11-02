/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prprpr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:46:07 $
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


#ifndef ARY_CPP_PRPRPR_HXX  // PRePRocessorPRocessing
#define ARY_CPP_PRPRPR_HXX



// Implemented in autodoc/source/parser/cpp/defdescr.cxx .

bool                CheckForOperator(
                        bool &              o_bStringify,
                        bool &              o_bConcatenate,
                        const String &     i_sTextItem );
void                Do_bConcatenate(
                        csv::StreamStr &    o_rText,
                        bool &              io_bConcatenate );
void                Do_bStringify_begin(
                        csv::StreamStr &    o_rText,
                        bool                i_bStringify );
void                Do_bStringify_end(
                        csv::StreamStr &    o_rText,
                        bool &              io_bStringify );
bool                HandleOperatorsBeforeTextItem(  /// @return true, if text item is done here
                        csv::StreamStr &    o_rText,
                        bool &              io_bStringify,
                        bool &              io_bConcatenate,
                        const String &     i_sTextItem );




#endif
