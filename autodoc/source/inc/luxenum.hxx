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

#ifndef UDM_LUXENUM_HXX
#define UDM_LUXENUM_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <map>
#include <algorithm>


namespace lux
{

typedef std::map< intt, String  > EnumValueMap;


template <class DIFF>
class Enum // : public Template_Base
{
  public:
    // TYPES
    typedef Enum< DIFF >	self;

    // LIFECYCLE
                        Enum(
                            DIFF				i_nValue,
                            const char *		i_sText )
                                                :	nValue(i_nValue) { Values_()[nValue] = i_sText;
                                                                       // Sequence_().insert(
                                                                       //		std::lower_bound( Sequence_().begin(), Sequence_().end(), i_nValue ),
                                                                       //		i_nValue );
                                                                     }
                        Enum(
                            DIFF				i_nValue )
                                                :	nValue(i_nValue) { ; }
                        Enum(
                            intt				i_nValue = 0 )
                                                :	nValue(i_nValue) { if ( NOT CheckIntt(i_nValue) ) { csv_assert(false); } }
                        Enum(
                            const self &		i_rEnum )
                                                :	nValue(i_rEnum.nValue) {;}

    self &				operator=(
                            DIFF				i_nValue )
                                                { nValue = i_nValue; return *this; }
    self &				operator=(
                            intt				i_nValue )
                                                { if ( CheckIntt(i_nValue) ) {nValue = DIFF(i_nValue);}
                                                  else {csv_assert(false);} return *this; }
    self &				operator=(
                            const self &		i_rEnum )
                                                { nValue = i_rEnum.nValue; return *this; }
                        operator DIFF() const 	{ return DIFF(nValue); }

    DIFF                operator()() const		{ return nValue; }
    const String  &		Text() const			{ return Values_()[nValue]; }

  private:
    static EnumValueMap &
                        Values_();
    bool				CheckIntt(
                            intt				i_nNumber )
                                                { return Values_().find(i_nNumber) != Values_().end(); }
    // DATA
    intt				nValue;
};




}   // namespace lux
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
