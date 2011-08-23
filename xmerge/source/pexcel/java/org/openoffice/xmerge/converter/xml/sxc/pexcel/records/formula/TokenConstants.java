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

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records.formula;

public interface TokenConstants {
        
    // Unary Operator Tokens
    public static final int TUPLUS      	= 0x12;
        public static final int TUMINUS         = 0x13;
        public static final int TPERCENT        = 0x14;
        public static final int TPAREN          = 0x15;
    // Binary Operator Tokens
    public static final int TADD        	= 0x03;
        public static final int TSUB            = 0x04;
        public static final int TMUL            = 0x05;
        public static final int TDIV            = 0x06;
        public static final int TPOWER          = 0x07;
        public static final int TCONCAT         = 0x08;
        
    //Logical operators
        public static final int TLESS		= 0x09;
        public static final int TLESSEQUALS	= 0x0A;
    public static final int TEQUALS		= 0x0B;
        public static final int TGTEQUALS	= 0x0C;
    public static final int TGREATER	= 0x0D;
        public static final int TNEQUALS	= 0x0E;        
        
    // Function Operator Tokens
        public static final int TFUNC           = 0x41;
        public static final int TFUNCVAR        = 0x42;        
        
    // Constant Operand Tokens
        public static final int TSTRING			= 0x17;
        public static final int TINT            = 0x1E;
        public static final int TNUM            = 0x1F;
    // Operand Tokens    
        public static final int TREF            = 0x44;
        public static final int TAREA           = 0x25;
        public static final int TNAME           = 0x23;
        public static final int TREF3D			= 0x3A;
        public static final int TAREA3D			= 0x3B;

    //
        public static final int TARGSEP         = 0x1001;
        public static final int TCLOSEPAREN 	= 0x1002;

    // Variable argument Functions
        // Math and Trig
        public static final int TSUM	    	= 0x04;
        public static final int TPRODUCT        = 0xB7;
        public static final int TSUMIF          = 0x0159; 
        public static final int TLOG            = 0x6D;  
        public static final int TRUNC           = 0xC5; 
        // Financial
        public static final int TDDB            = 0x90;  
        public static final int TFV             = 0x39;  
        public static final int TIRR            = 0x3E;  
        public static final int TNPER           = 0x3A; 
        public static final int TNPV            = 0x0B; 
        public static final int TPMT            = 0x3B; 
        public static final int TPV             = 0x38; 
        public static final int TRATE           = 0x3C;
        // Statistical
        public static final int TAVERAGE        = 0x05; 
        public static final int TCOUNT          = 0x00;
        public static final int TCOUNTA         = 0xA9;
        public static final int TMAX            = 0x07; 
        public static final int TMIN            = 0x06; 
        public static final int TSTDEV          = 0x0C; 
        public static final int TSTDEVP         = 0xC1; 
        public static final int TVAR            = 0x2E; 
        public static final int TVARP           = 0xC2; 
        // Lookup
        public static final int TCHOOSE         = 0x64; 
        public static final int THLOOKUP        = 0x65; 
        public static final int TINDEX          = 0x1D; 
        public static final int TMATCH          = 0x40; 
        public static final int TVLOOKUP        = 0x66; 
        // Text
        public static final int TRIGHT          = 0x74; 
        public static final int TSUBSTITUTE     = 0x78; 
        public static final int TFIND           = 0x7c; 
        public static final int TLEFT           = 0x73; 
        // Logical
        public static final int TAND            = 0x24; // 42
        public static final int TIF             = 0x01; // 42
        public static final int TOR             = 0x25; // 42

    // Fixed argument Functions
        // Math and Trig
        public static final int TABS            = 0x18;
        public static final int TACOS           = 0x63;
        public static final int TASIN           = 0x62;
        public static final int TATAN           = 0x12;
        public static final int TATAN2          = 0x61;
        public static final int TCOS            = 0x10;
        public static final int TSIN            = 0x0F;
        
        public static final int TCOUNTIF        = 0x015A;
        public static final int TDEGREES        = 0x0157;
        public static final int TEXP            = 0x15;
        public static final int TFACT           = 0xB8;
        public static final int TINTE           = 0x19;
        public static final int TLN             = 0x16;
        
        public static final int TLOG10          = 0x17;
        public static final int TMOD            = 0x27;
        public static final int TPI             = 0x13;
        
        public static final int TPOWERF         = 0x0151;
        public static final int TRADIANS        = 0x0156;
        public static final int TRAND           = 0x3F;
        public static final int TROUND          = 0x1B;
        public static final int TSQRT           = 0x14;               
        public static final int TTAN            = 0x11; 
        
        public static final int TSLN            = 0x8E; 
        public static final int TSYD            = 0x8F; 
        
        // Date and Time
        public static final int TDATE           = 0x41; 
        public static final int TDATEVALUE      = 0x8C; 
        public static final int TDAY            = 0x43;
        public static final int THOUR           = 0x47; 
        public static final int TMINUTE         = 0x48; 
        public static final int TMONTH          = 0x44; 
        public static final int TNOW            = 0x4A; 
        public static final int TSECOND         = 0x49; 
        public static final int TTIME           = 0x42; 
        public static final int TTIMEVALUE      = 0x8D;
        public static final int TYEAR           = 0x45;         
        // Statistical
        public static final int TCOUNTBLANK     = 0x015B ; 
        // lookup
        public static final int TCOLUMNS        = 0x4D; 
        public static final int TROWS           = 0x4C; 
        // Database
        public static final int TDAVAERAGE      = 0x2A; 
        public static final int TDCOUNT         = 0x28; 
        public static final int TDCOUNTA        = 0xC7; 
        public static final int TDGET           = 0xEB; 
        public static final int TDMAX           = 0x2C; 
        public static final int TDMIN           = 0x2B;
        public static final int TDPRODUCT       = 0xBD; 
        public static final int TDSTDEV         = 0x2D; 
        public static final int TDSTDEVP        = 0xC3;
        public static final int TDSUM           = 0x29; 
        public static final int TDVAR           = 0x2F; 
        public static final int TDVARP          = 0xC4; 
        // Text
        public static final int TEXACT          = 0x75; 
        public static final int TLEN            = 0x20; 
        public static final int TLOWER          = 0x70; 
        public static final int TMID            = 0x1F; // ??????
        public static final int TPROPER         = 0x72; 
        public static final int TREPLACE        = 0x77; 
        public static final int TREPT           = 0x1E; 
        public static final int TT              = 0x82; 
        public static final int TRIM            = 0x76; 
        public static final int TUPPER          = 0x71; 
        public static final int TVALUE          = 0x21; 
        // Logical
        public static final int TFALSE          = 0x23; 
        public static final int TNOT            = 0x26; 
        public static final int TTRUE           = 0x22; 
        // Informational
        public static final int TERRORTYPE      = 0x05; 
        public static final int TISBLANK        = 0x81;
        public static final int TISERR          = 0x7E; 
        public static final int TISERROR        = 0x03; 
        public static final int TISLOGICAL      = 0xC6; 
        public static final int TISNA           = 0x02; 
        public static final int TISNONTEXT      = 0xBE; 
        public static final int TISNUMBER       = 0x80; 
        public static final int TISTEXT         = 0x7F; 
        public static final int TN              = 0x83; 
        public static final int TNA             = 0x0A; 	
}
