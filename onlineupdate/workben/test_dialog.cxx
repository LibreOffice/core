#include "progressui.h"

#if defined(_WIN32)
#include "progressui_win.cxx"
#else
#include "progressui_gtk.cxx"
#endif

#include <thread>
#include <chrono>
#include <iostream>

void func()
{
    for (int i = 0; i <= 100; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        UpdateProgressUI(i);
    }
    QuitProgressUI();
}

int NS_main(int argc, NS_tchar** argv)
{
    InitProgressUI(&argc, &argv);
    std::thread a(func);
    /*
    volatile bool b = false;
    do
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    while (!b);
    */
    int result = ShowProgressUI();
    std::cout << result << std::endl;
    a.join();
    return 0;
}
