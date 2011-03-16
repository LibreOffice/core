
/*
 * Urgh - checked in temporarily - until we merge m100 and
 * can generate .cxx files nicely
 */

#ifndef _MSC_VER
#warning "checked in generated file - urgh"
#endif

//Do not edit manually, generated from bestreversemap.cxx
rtl_TextEncoding getBestMSEncodingByChar(sal_Unicode c)
{
    if (c < 0x80)
        return RTL_TEXTENCODING_MS_1252;
    if (c < 0xa0)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x100)
        return RTL_TEXTENCODING_MS_1252;
    if (c < 0x102)
        return RTL_TEXTENCODING_MS_1257;
    if (c < 0x108)
        return RTL_TEXTENCODING_MS_1250;
    if (c < 0x10c)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x112)
        return RTL_TEXTENCODING_MS_1250;
    if (c < 0x114)
        return RTL_TEXTENCODING_MS_1257;
    if (c < 0x116)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x11a)
        return RTL_TEXTENCODING_MS_1257;
    if (c < 0x11c)
        return RTL_TEXTENCODING_MS_1250;
    if (c < 0x11e)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x120)
        return RTL_TEXTENCODING_MS_1254;
    if (c < 0x122)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x124)
        return RTL_TEXTENCODING_MS_1257;
    if (c < 0x126)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x128)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x12a)
        return RTL_TEXTENCODING_MS_1258;
    if (c < 0x12c)
        return RTL_TEXTENCODING_MS_1257;
    if (c < 0x12e)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x130)
        return RTL_TEXTENCODING_MS_1257;
    if (c < 0x132)
        return RTL_TEXTENCODING_MS_1254;
    if (c < 0x134)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x136)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x138)
        return RTL_TEXTENCODING_MS_1257;
    if (c < 0x139)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x13b)
        return RTL_TEXTENCODING_MS_1250;
    if (c < 0x13d)
        return RTL_TEXTENCODING_MS_1257;
    if (c < 0x13f)
        return RTL_TEXTENCODING_MS_1250;
    if (c < 0x143)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x147)
        return RTL_TEXTENCODING_MS_1257;
    if (c < 0x149)
        return RTL_TEXTENCODING_MS_1250;
    if (c < 0x14c)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x14e)
        return RTL_TEXTENCODING_MS_1257;
    if (c < 0x150)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x152)
        return RTL_TEXTENCODING_MS_1250;
    if (c < 0x154)
        return RTL_TEXTENCODING_MS_1252;
    if (c < 0x156)
        return RTL_TEXTENCODING_MS_1250;
    if (c < 0x158)
        return RTL_TEXTENCODING_MS_1257;
    if (c < 0x15c)
        return RTL_TEXTENCODING_MS_1250;
    if (c < 0x15e)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x166)
        return RTL_TEXTENCODING_MS_1250;
    if (c < 0x168)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x16a)
        return RTL_TEXTENCODING_MS_1258;
    if (c < 0x16c)
        return RTL_TEXTENCODING_MS_1257;
    if (c < 0x16e)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x172)
        return RTL_TEXTENCODING_MS_1250;
    if (c < 0x174)
        return RTL_TEXTENCODING_MS_1257;
    if (c < 0x178)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x179)
        return RTL_TEXTENCODING_MS_1252;
    if (c < 0x17f)
        return RTL_TEXTENCODING_MS_1250;
    if (c < 0x192)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x193)
        return RTL_TEXTENCODING_MS_1252;
    if (c < 0x1a0)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x1a2)
        return RTL_TEXTENCODING_MS_1258;
    if (c < 0x1af)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x1b1)
        return RTL_TEXTENCODING_MS_1258;
    if (c < 0x1ce)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x1cf)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x1d0)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x1d1)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x1d2)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x1d3)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x1d4)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x1d5)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x1d6)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x1d7)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x1d8)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x1d9)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x1da)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x1db)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x1dc)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x1dd)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x251)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x252)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x261)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x262)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x2c6)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2c7)
        return RTL_TEXTENCODING_MS_1252;
    if (c < 0x2c8)
        return RTL_TEXTENCODING_MS_1250;
    if (c < 0x2c9)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2cc)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x2cd)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2ce)
        return RTL_TEXTENCODING_MS_950;
    if (c < 0x2d0)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2d1)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x2d8)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2dc)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x2dd)
        return RTL_TEXTENCODING_MS_1252;
    if (c < 0x2de)
        return RTL_TEXTENCODING_MS_1250;
    if (c < 0x300)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x302)
        return RTL_TEXTENCODING_MS_1258;
    if (c < 0x303)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x304)
        return RTL_TEXTENCODING_MS_1258;
    if (c < 0x309)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x30a)
        return RTL_TEXTENCODING_MS_1258;
    if (c < 0x323)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x324)
        return RTL_TEXTENCODING_MS_1258;
    if (c < 0x384)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x387)
        return RTL_TEXTENCODING_MS_1253;
    if (c < 0x388)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x38b)
        return RTL_TEXTENCODING_MS_1253;
    if (c < 0x38c)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x38d)
        return RTL_TEXTENCODING_MS_1253;
    if (c < 0x38e)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x3a2)
        return RTL_TEXTENCODING_MS_1253;
    if (c < 0x3a3)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x3cf)
        return RTL_TEXTENCODING_MS_1253;
    if (c < 0x401)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x40d)
        return RTL_TEXTENCODING_MS_1251;
    if (c < 0x40e)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x450)
        return RTL_TEXTENCODING_MS_1251;
    if (c < 0x451)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x45d)
        return RTL_TEXTENCODING_MS_1251;
    if (c < 0x45e)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x460)
        return RTL_TEXTENCODING_MS_1251;
    if (c < 0x490)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x492)
        return RTL_TEXTENCODING_MS_1251;
    if (c < 0x5b0)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x5ba)
        return RTL_TEXTENCODING_MS_1255;
    if (c < 0x5bb)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x5c4)
        return RTL_TEXTENCODING_MS_1255;
    if (c < 0x5d0)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x5eb)
        return RTL_TEXTENCODING_MS_1255;
    if (c < 0x5f0)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x5f5)
        return RTL_TEXTENCODING_MS_1255;
    if (c < 0x60c)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x60d)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0x61b)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x61c)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0x61f)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x620)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0x621)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x63b)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0x640)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x653)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0x679)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x67a)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0x67e)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x67f)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0x686)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x687)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0x688)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x689)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0x691)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x692)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0x698)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x699)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0x6a9)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x6aa)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0x6af)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x6b0)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0x6ba)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x6bb)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0x6be)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x6bf)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0x6c1)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x6c2)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0x6d2)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x6d3)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0xe01)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0xe3b)
        return RTL_TEXTENCODING_MS_874;
    if (c < 0xe3f)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0xe5c)
        return RTL_TEXTENCODING_MS_874;
    if (c < 0x1e3f)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x1e40)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x1ea0)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x1efa)
        return RTL_TEXTENCODING_MS_1258;
    if (c < 0x200c)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2010)
        return RTL_TEXTENCODING_MS_1256;
    if (c < 0x2011)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2013)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2017)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x2018)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x201b)
        return RTL_TEXTENCODING_MS_1252;
    if (c < 0x201c)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x201f)
        return RTL_TEXTENCODING_MS_1252;
    if (c < 0x2020)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2023)
        return RTL_TEXTENCODING_MS_1252;
    if (c < 0x2025)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2028)
        return RTL_TEXTENCODING_MS_950;
    if (c < 0x2030)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2031)
        return RTL_TEXTENCODING_MS_1252;
    if (c < 0x2032)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2034)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2035)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2036)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x2039)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x203b)
        return RTL_TEXTENCODING_MS_1252;
    if (c < 0x203c)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x203e)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x203f)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2074)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2075)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x207f)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2080)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x2081)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2085)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x20a9)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x20aa)
        return RTL_TEXTENCODING_MS_1361;
    if (c < 0x20ab)
        return RTL_TEXTENCODING_MS_1255;
    if (c < 0x20ad)
        return RTL_TEXTENCODING_MS_1258;
    if (c < 0x2103)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2104)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2105)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2106)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x2109)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x210a)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x2113)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2114)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x2116)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2117)
        return RTL_TEXTENCODING_MS_1251;
    if (c < 0x2121)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2123)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x2126)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2127)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x212b)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x212c)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2153)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2155)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x215b)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x215f)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x2160)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x216c)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x2170)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x217a)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2190)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x219a)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x21d2)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x21d3)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x21d4)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x21d5)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2200)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2201)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2202)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2204)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2207)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2209)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x220b)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x220c)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x220f)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2210)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x2211)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2213)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2215)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2216)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x221a)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x221b)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x221d)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2221)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2223)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2224)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x2225)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2226)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2227)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x222d)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x222e)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x222f)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2234)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2238)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x223c)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x223e)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x2248)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2249)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x224c)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x224d)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x2252)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2253)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2260)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2262)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2264)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2268)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x226a)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x226c)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x226e)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2270)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x2282)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2284)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2286)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2288)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2295)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2296)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x2299)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x229a)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x22a5)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x22a6)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x22bf)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x22c0)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x22ef)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x22f0)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x2312)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2313)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2460)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2474)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x249c)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x24b6)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x24d0)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x24ea)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x2500)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x254c)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x2550)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2575)
        return RTL_TEXTENCODING_MS_950;
    if (c < 0x2581)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2590)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x2592)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2593)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x2596)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x25a0)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x25a2)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x25a3)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x25aa)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x25b2)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x25b4)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x25b6)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x25b8)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x25bc)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x25be)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x25c0)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x25c2)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x25c6)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x25c9)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x25cb)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x25cc)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x25ce)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x25d2)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x25e2)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x25e6)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x25ef)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x25f0)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2605)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2607)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x2609)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x260a)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x260e)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2610)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x261c)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x261d)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x261e)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x261f)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x2640)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2643)
        return RTL_TEXTENCODING_MS_950;
    if (c < 0x2660)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2662)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x2663)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2666)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x2667)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x266b)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x266c)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x266e)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x266f)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x2670)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x3000)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x3004)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x3005)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x3018)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x301c)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x301f)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x3020)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x3021)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x302a)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x3041)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x3094)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x309b)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x309f)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x30a1)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x30f7)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x30fb)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x30ff)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x3105)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x312a)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x3131)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x31ac)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x31ef)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x31ff)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x3200)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x321d)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x3220)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x322a)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x3231)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x3233)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x3239)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x323a)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x3260)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x327c)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x327f)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x3280)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x32a3)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x32a4)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x32a9)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x3303)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x3304)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x330d)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x330e)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x3314)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x3315)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x3318)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x3319)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x3322)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x3324)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x3326)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x3328)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x332b)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x332c)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x3336)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x3337)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x333b)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x333c)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x3349)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x334b)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x334d)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x334e)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x3351)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x3352)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x3357)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x3358)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x337b)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x337f)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x3380)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x3385)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x3388)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x33cb)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x33cd)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x33ce)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0x33cf)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x33d1)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x33d3)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x33d4)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x33d5)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x33d6)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0x33d7)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x33d8)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x33d9)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x33db)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x33de)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0x4e00)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0x9fa6)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0xac00)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0xd7a4)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0xe000)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0xf849)
        return RTL_TEXTENCODING_MS_950;
    if (c < 0xf900)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0xfa0c)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0xfa10)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0xfa2e)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0xfe30)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0xfe32)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0xfe33)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0xfe45)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0xfe49)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0xfe53)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0xfe54)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0xfe58)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0xfe59)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0xfe67)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0xfe68)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0xfe6c)
        return RTL_TEXTENCODING_MS_936;
    if (c < 0xff01)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0xff5f)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0xff61)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0xffa0)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0xffe0)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0xffe6)
        return RTL_TEXTENCODING_MS_932;
    if (c < 0xffe7)
        return RTL_TEXTENCODING_MS_949;
    if (c < 0xfffd)
        return RTL_TEXTENCODING_DONTKNOW;
    if (c < 0xfffe)
        return RTL_TEXTENCODING_MS_950;
    return RTL_TEXTENCODING_DONTKNOW;
}
