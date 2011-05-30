#include <rtftypes.hxx>

namespace writerfilter {
namespace rtftok {

// See RTF spec v1.9.1, page 19
RTFEncoding aRTFEncodings[] = {
    // charset  codepage    Windows / Mac name
    {0, 1252}, // ANSI
    {1, 0}, // Default
    {2, 42}, // Symbol
    {77, 10000}, // Mac Roman
    {78, 10001}, // Mac Shift Jis
    {79, 10003}, // Mac Hangul
    {80, 10008}, // Mac GB2312
    {81, 10002}, // Mac Big5
    {83, 10005}, // Mac Herbrew
    {84, 10004}, // Mac Arabic
    {85, 10006}, // Mac Greek
    {86, 10081}, // Mac Turkish
    {87, 10021}, // Mac Thai
    {88, 10029}, // Mac East Europe
    {89, 10007}, // Mac Russian
    {128, 932}, // Shift JIS
    {129, 949}, // Hangul
    {130, 1361}, // Johab
    {134, 936}, // GB2312
    {136, 950}, // Big5
    {161, 1253}, // Greek
    {162, 1254}, // Turkish
    {163, 1258}, // Viatnamese
    {177, 1255}, // Herbrew
    {178, 1256}, // Arabic
    {186, 1257}, // Baltic
    {204, 1251}, // Russian
    {222, 874}, // Thai
    {238, 1250}, // Eastern European
    {254, 437}, // PC 437
    {255, 850} // OEM
};

int nRTFEncodings = sizeof(aRTFEncodings) / sizeof(RTFEncoding);

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
