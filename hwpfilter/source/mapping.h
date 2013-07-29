/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef __MAPPING_H__
#define __MAPPING_H__

#include <string.h>
#include <stdio.h>
#include <sal/macros.h>

#include <hwplib.h>


struct FormulaEntry{
     const char *tex;
     hchar ucs;
};

// empty square
#define DEFAULT_VALUE   0x25a1
const struct FormulaEntry FormulaMapTab[] = {
/* Capital Greek */
{"Alpha", 0x0391},
{"Beta", 0x0392},
{"Gamma", 0x0393},
{"Delta", 0x0394},
{"Epsilon", 0x0395},
{"Zeta", 0x0396},
{"Eta", 0x0397},
{"Theta", 0x0398},
{"Iota", 0x0399},
{"Kappa", 0x039a},
{"Lambda",0x039b },
{"Mu", 0x039c},
{"Nu", 0x039d},
{"Xi", 0x039e},
{"Omicron", 0x039f},
{"Pi", 0x03a0},
{"Rho", 0x03a1},
{"Sigma", 0x03a3},
{"Tau", 0x03a4},
{"Upsilon", 0x03a5},
{"Phi", 0x03a6},
{"Chi", 0x03a7},
{"Psi", 0x03a8},
{"Omega", 0x03a9},

/* Small Greek */
{"alpha", 0x03b1},
{"beta", 0x03b2},
{"gamma", 0x03b3},
{"delta", 0x03b4},
{"epsilon", 0x03b5},
{"zeta", 0x03b6},
{"eta", 0x03b7},
{"theta", 0x03b8},
{"iota", 0x03b9},
{"kappa", 0x03ba},
{"lambda",0x03bb },
{"mu", 0x03bc},
{"nu", 0x03bd},
{"xi", 0x03be},
{"omicron", 0x03bf},
{"pi", 0x03c0},
{"rho", 0x03c1},
{"varrho", 0},
{"sigma", 0x03c3},
{"tau", 0x03c4},
{"upsilon", 0x03c5},
{"phi", 0x03d5},
{"chi", 0x03c7},
{"psi", 0x03c8},
{"omega", 0x03c9},

/* General Symbol */
{"aleph", 0x2135},
{"hbar", 0x045b},
{"imath", 0x2129},
{"jmath", 0x2110},
{"mho", 0x2127},
{"ell", 0x2113},
{"wp", 0x2118},
{"imag", 0x2111},
{"angstrom", 0x212b},
{"vartheta", 0x03d1},
{"varpi", 0x03d6},
{"varsigma", 0x04aa},
{"varupsilon", 0x03d2},
{"varphi", 0x03c6},


// Big Symbol
{"sum", 0x2211},
{"smallsum", 0x03a3},
{"prod", 0x220f},
{"amalg", 0x2210},
{"int", 0x222b},
{"oint", 0x222e},
{"cap", 0x22c2},
{"cup", 0x22c3},
{"uplus", 0x2282},
{"vee", 0x22c1},
{"wedge", 0x22c0},
{"bigcap", 0x22c2},
{"bigcup", 0x22c3},
{"bigsqcap", 0x2293},
{"bigsqcup", 0x2294},
{"bigoplus", 0x2295},
{"bigominus", 0x2296},
{"bigotimes", 0x2297},
{"bigodiv", 0x2298},
{"bigodot", 0x2299},
{"oplus", 0x2295},
{"ominus", 0x2296},
{"otimes", 0x2297},
{"oslash", 0x2298},
{"odot", 0x2299},
{"bigvee", 0x22c1},
{"bigwedge", 0x22c0},
{"subset", 0x2282},
{"superset", 0x2283},
{"supset", 0x2283},
{"subseteq", 0x2286},
{"supseteq", 0x2287},
{"in", 0x2208},
{"ni", 0x220b},
{"notin", 0x2209},
{"notni", 0x220c},
{"leq", 0x2264},
{"geq", 0x2265},
{"sqsubset", 0x228f},
{"sqsupset", 0x2290},
{"sqsubseteq", 0x2291},
{"sqsupseteq", 0x2292},
{"mlt", 0x2292}, // much less than
{"mgt", 0x2292}, // much greater than
{"vmlt", 0x22d8}, // much less than
{"vmgt", 0x22d9}, // much greater than
{"prec", 0x227a},
{"succ", 0x227b},
{"biguplus", 0x228e},

// Binary Operator
{"pm", 0x00b1},
{"mp", 0x2213},
{"times", 0x00d7},
{"div", 0x00f7},
{"circ", 0x2218},
{"cdot", 0x2219},
{"bullet", 0x2022},
{"deg", 0x00b0},
{"ast", 0x002a},
{"star", 0x2205},
{"bigcirc", 0x25cb},
{"emptyset", 0x2205},
{"therefore", 0x2234},
{"because", 0x2235},
{"identical", 0x2237},
{"exists", 0x2203},
{"noteq", 0x2260}, // !=
{"neq", 0x2260},
{"doteq", 0x2250},
{"image", 0x2252},
{"reimage", 0x2253},
{"sim", 0x223c},
{"approx", 0x2248},
{"simeq", 0x2243},
{"cong", 0x2245},
{"equiv", 0x2261},
{"asymp", 0x224d},
{"iso", 0x224e},
{"diamond", 0x22c4},
{"dsum", 0x2214}, // add
{"forall", 0x2200},
{"prime", DEFAULT_VALUE}, // '
{"partial", 0x2202},
{"infty", 0x221e},
{"propto", 0x221d},
{"xor", 0x22bb},
{"del", 0x2207},
{"dagger", 0x2020},
{"ddagger", 0x2021},
{"LNOT", DEFAULT_VALUE},

// Arrows
{"leftarrow", 0x2190},
{"uparrow", 0x2191},
{"rightarrow", 0x2192},
{"downarrow", 0x2193},
{"Leftarrow", 0x21d0},
{"Uparrow", 0x21d1},
{"Rightarrow", 0x21d2},
{"Downarrow", 0x21d3},
{"updownarrow", 0x2195},
{"Updownarrow", 0x21d5},
{"leftrightarrow", 0x2194},
{"Leftrightarrow", 0x21d4},
{"nwarrow", 0x2196},
{"searrow", 0x2198},
{"nearrow", 0x2197},
{"swarrow", 0x2199},
{"hookleft", 0x21a9},
{"hookleftarrow", 0x21a9},
{"hookright", 0x21aa},
{"hookrightarrow", 0x21aa},
{"mapsto", 0x21a6},
{"vert", 0x2223},
{"dline", 0x2225},

{"cdots", 0x22ef},
{"ldots", 0x2026},
{"vdots", 0x22ee},
{"ddots", 0x22f1},
{"triangle", 0x2206},
{"triangled", 0x2207},
{"angle", 0x2220},
{"msangle", 0x2221},
{"sangle", 0x2222},
{"rtangle", 0x22be},
{"vdash", 0x22a2},
{"dashv", 0x22a3},
{"bot", 0x22a5},
{"top", 0x22a4},
{"models", 0x22a8},

{"coprod", DEFAULT_VALUE},
{"varepsilon", DEFAULT_VALUE},
{"setminus", DEFAULT_VALUE},
{"sqcap", DEFAULT_VALUE},
{"sqcup", DEFAULT_VALUE},
{"wr", DEFAULT_VALUE},
{"bigtriangleup", DEFAULT_VALUE},
{"bigtriangledown", DEFAULT_VALUE},
{"triangleleft", DEFAULT_VALUE},
{"triangleright", DEFAULT_VALUE},
{"lhd", DEFAULT_VALUE},
{"rhd", DEFAULT_VALUE},
{"unlhd", DEFAULT_VALUE},
{"unrhd", DEFAULT_VALUE},
{"nabla", DEFAULT_VALUE},
{"surd", DEFAULT_VALUE},
{"Box", DEFAULT_VALUE},
{"Diamond", DEFAULT_VALUE},
{"neg", DEFAULT_VALUE},
{"flat", DEFAULT_VALUE},
{"natural", DEFAULT_VALUE},
{"sharp", DEFAULT_VALUE},
{"clubsuit", DEFAULT_VALUE},
{"diamondsuit", DEFAULT_VALUE},
{"heartsuit", DEFAULT_VALUE},
{"spadesuit", DEFAULT_VALUE},
{"Re", DEFAULT_VALUE},
{"Im", DEFAULT_VALUE},
{"S", DEFAULT_VALUE},
{"P", DEFAULT_VALUE},
{"smallint", DEFAULT_VALUE},
{"backslash", DEFAULT_VALUE},

// Relation Operator
{"le", DEFAULT_VALUE},
{"ge", DEFAULT_VALUE},
{"perp", DEFAULT_VALUE},
{"preceq", DEFAULT_VALUE},
{"succeq", DEFAULT_VALUE},
{"mid", DEFAULT_VALUE},
{"ll", DEFAULT_VALUE},
{"gg", DEFAULT_VALUE},
{"parallel", DEFAULT_VALUE},
{"bowtie", DEFAULT_VALUE},
{"Join", DEFAULT_VALUE},
{"smile", DEFAULT_VALUE},
{"frown", DEFAULT_VALUE},
{"not", DEFAULT_VALUE},
{"mapsto", DEFAULT_VALUE},

// Arrows
{"to", DEFAULT_VALUE},

{"leftharpoonup", DEFAULT_VALUE},
{"leftharpoondown", DEFAULT_VALUE},
{"longleftarrow", DEFAULT_VALUE},
{"Longleftarrow", DEFAULT_VALUE},
{"longrightarrow", DEFAULT_VALUE},
{"Longrightarrow", DEFAULT_VALUE},
{"longleftrightarrow", DEFAULT_VALUE},
{"Longleftrightarrow", DEFAULT_VALUE},
{"longmapsto", DEFAULT_VALUE},
{"rightharpoonup", DEFAULT_VALUE},
{"rightharpoondown", DEFAULT_VALUE},

// Delimeter
{"(", DEFAULT_VALUE},
{")", DEFAULT_VALUE},
{"[", DEFAULT_VALUE},
{"]", DEFAULT_VALUE},
{"{", DEFAULT_VALUE},
{"}", DEFAULT_VALUE},
{"lfloor", DEFAULT_VALUE},
{"rfloor", DEFAULT_VALUE},
{"lceil", DEFAULT_VALUE},
{"rceil", DEFAULT_VALUE},
{"langle", DEFAULT_VALUE},
{"rangle", DEFAULT_VALUE},
{"mid", DEFAULT_VALUE},

// Large Delimeter
{"rmoustache", DEFAULT_VALUE},
{"lmoustache", DEFAULT_VALUE},
{"rgroup", DEFAULT_VALUE},
{"lgroup", DEFAULT_VALUE},
{"arrowvert", DEFAULT_VALUE},
{"Arrowvert", DEFAULT_VALUE},
{"bracevert", DEFAULT_VALUE},

// Accent
{"hat", DEFAULT_VALUE},
{"breve", DEFAULT_VALUE},
{"grave", DEFAULT_VALUE},
{"bar", DEFAULT_VALUE},
{"ddot", DEFAULT_VALUE},
{"check", DEFAULT_VALUE},
{"acute", DEFAULT_VALUE},
{"tilde", DEFAULT_VALUE},
{"dot", DEFAULT_VALUE},
{"vec", DEFAULT_VALUE},

// Decoration
{"overline", DEFAULT_VALUE},
{"underline", DEFAULT_VALUE},
{"overbrace", DEFAULT_VALUE},
{"underbrace", DEFAULT_VALUE},
{"widehat", DEFAULT_VALUE},
{"widetilde", DEFAULT_VALUE},
{"overleftarrow", DEFAULT_VALUE},
{"overrightarrow", DEFAULT_VALUE},

// Space Symbol
{",", DEFAULT_VALUE},
{":", DEFAULT_VALUE},
{";", DEFAULT_VALUE},
{"!", DEFAULT_VALUE},
{"quad", DEFAULT_VALUE},
{"qquad", DEFAULT_VALUE}, // double size of character

};

#ifndef DEBUG
hchar_string getMathMLEntity(const char *tex)
{
     static const size_t tabSize = sizeof(FormulaMapTab) / sizeof(FormulaMapTab[0]);

     hchar_string buf;
     for (size_t i = 0 ; i < tabSize ; i++) {
          if( !strcmp(tex, FormulaMapTab[i].tex ) ) {
                buf.push_back(FormulaMapTab[i].ucs);
                return buf;
          }
     }

     size_t const len = strlen(tex);
     for (size_t i = 0 ; i < len ; i++)
     {
         buf.push_back(tex[i]);
     }
     return buf;
}

#else
::std::string getMathMLEntity(const char *tex)
{
     ::std::string buf;
     buf.append(tex);
     return buf;
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
