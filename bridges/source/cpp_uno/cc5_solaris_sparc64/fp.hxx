/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-04-24 16:26:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2008 by Sun Microsystems, Inc.
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

#ifndef INCLUDED_BRIDGES_SOURCE_CPP_UNO_CC5_SOLARIS_SPARC64_FP_HXX
#define INCLUDED_BRIDGES_SOURCE_CPP_UNO_CC5_SOLARIS_SPARC64_FP_HXX

#include "sal/config.h"

extern "C" {

void fp_loadf0(float *);
void fp_loadf1(float *);
void fp_loadf2(float *);
void fp_loadf3(float *);
void fp_loadf4(float *);
void fp_loadf5(float *);
void fp_loadf6(float *);
void fp_loadf7(float *);
void fp_loadf9(float *);
void fp_loadf11(float *);
void fp_loadf13(float *);
void fp_loadf15(float *);
void fp_loadf17(float *);
void fp_loadf19(float *);
void fp_loadf21(float *);
void fp_loadf23(float *);
void fp_loadf25(float *);
void fp_loadf27(float *);
void fp_loadf29(float *);
void fp_loadf31(float *);

void fp_storef0(float *);
void fp_storef1(float *);
void fp_storef2(float *);
void fp_storef3(float *);
void fp_storef4(float *);
void fp_storef5(float *);
void fp_storef6(float *);
void fp_storef7(float *);
void fp_storef9(float *);
void fp_storef11(float *);
void fp_storef13(float *);
void fp_storef15(float *);
void fp_storef17(float *);
void fp_storef19(float *);
void fp_storef21(float *);
void fp_storef23(float *);
void fp_storef25(float *);
void fp_storef27(float *);
void fp_storef29(float *);
void fp_storef31(float *);

void fp_loadd0(double *);
void fp_loadd2(double *);
void fp_loadd4(double *);
void fp_loadd6(double *);
void fp_loadd8(double *);
void fp_loadd10(double *);
void fp_loadd12(double *);
void fp_loadd14(double *);
void fp_loadd16(double *);
void fp_loadd18(double *);
void fp_loadd20(double *);
void fp_loadd22(double *);
void fp_loadd24(double *);
void fp_loadd26(double *);
void fp_loadd28(double *);
void fp_loadd30(double *);

void fp_stored0(double *);
void fp_stored2(double *);
void fp_stored4(double *);
void fp_stored6(double *);
void fp_stored8(double *);
void fp_stored10(double *);
void fp_stored12(double *);
void fp_stored14(double *);
void fp_stored16(double *);
void fp_stored18(double *);
void fp_stored20(double *);
void fp_stored22(double *);
void fp_stored24(double *);
void fp_stored26(double *);
void fp_stored28(double *);
void fp_stored30(double *);

}

#endif
