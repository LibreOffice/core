#include <iostream>
#include "LibreOfficeKit/LibreOfficeKit.hxx"

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: lokconvert <input_file_path> <output_file_path>\n";
        return 1;
    }
    const char* input = argv[1];
    const char* output = argv[2];

    lok::Office* llo = NULL;
    try
    {
        const char* lo_bin_dir = std::getenv("OO_SDK_URE_BIN_DIR");
        llo = lok::lok_cpp_init(lo_bin_dir);
        if (!llo)
        {
            std::cerr << "Error: could not initialise LibreOfficeKit\n";
            return 1;
        }

        lok::Document* lodoc = llo->documentLoad(input, NULL /* options */);
        if (!lodoc)
        {
            std::cerr << "Error: could not load document: " << llo->getError() << "\n";
            return 1;
        }

        if (!lodoc->saveAs(output, "pdf", NULL /* options */))
        {
            std::cerr << "Error: could not export document: " << llo->getError() << "\n";
            return 1;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: LibreOfficeKit exception: " << e.what() << "\n";
        return 1;
    }

    std::cerr << "Success!\n";
    return 0;
}
