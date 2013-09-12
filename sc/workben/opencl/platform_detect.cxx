#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>

#include "clew.h"

using namespace std;

int main()
{
    const char* dllname = "OpenCL.dll"; // from GPU driver.
//  const char* dllname = "amdocl.dll"; // from AMD SDK
//  const char* dllname = "intelocl.dll"; // from Intel SDK
    int status = clewInit(dllname);
    if (status < 0)
    {
        cout << "failed to load" << endl;
        return EXIT_FAILURE;
    }

    cout << "OpenCL.dll loaded successfully." << endl;

    cl_uint platformCount = 0;
    clGetPlatformIDs(0, NULL, &platformCount);

    cout << "number of platforms: " << platformCount << endl;
    vector<cl_platform_id> platformIDs(platformCount);
    if (clGetPlatformIDs(platformCount, &platformIDs[0], NULL) != CL_SUCCESS)
    {
        cout << "failed to get platform IDs" << endl;
        return EXIT_FAILURE;
    }

    for (size_t i = 0, n = platformIDs.size(); i < n; ++i)
    {
        cout << "* platform (ID=" << platformIDs[i] << ")" << endl;
        string param(100, '\0');
        if (clGetPlatformInfo(platformIDs[i], CL_PLATFORM_PROFILE, param.size(), &param[0], NULL) == CL_SUCCESS)
            cout << "  profile: " << param.c_str() << endl;
        if (clGetPlatformInfo(platformIDs[i], CL_PLATFORM_VERSION, param.size(), &param[0], NULL) == CL_SUCCESS)
            cout << "  version: " << param.c_str() << endl;
        if (clGetPlatformInfo(platformIDs[i], CL_PLATFORM_NAME, param.size(), &param[0], NULL) == CL_SUCCESS)
            cout << "  name: " << param.c_str() << endl;
        if (clGetPlatformInfo(platformIDs[i], CL_PLATFORM_VENDOR, param.size(), &param[0], NULL) == CL_SUCCESS)
            cout << "  vendor: " << param.c_str() << endl;
        if (clGetPlatformInfo(platformIDs[i], CL_PLATFORM_EXTENSIONS, param.size(), &param[0], NULL) == CL_SUCCESS)
            cout << "  extensions: " << param.c_str() << endl;

        cl_uint deviceCount = 0;
        clGetDeviceIDs(platformIDs[i], CL_DEVICE_TYPE_ALL, 0, NULL, &deviceCount);
        cout << "  number of devices: " << deviceCount << endl;

        vector<cl_device_id> deviceIDs(deviceCount);
        if (clGetDeviceIDs(platformIDs[i], CL_DEVICE_TYPE_ALL, deviceCount, &deviceIDs[0], NULL) != CL_SUCCESS)
            continue;

        for (size_t j = 0; j < deviceIDs.size(); ++j)
        {
            cout << "  * device (ID=" << deviceIDs[j] << ")" << endl;
            if (clGetDeviceInfo(deviceIDs[j], CL_DEVICE_VENDOR, param.size(), &param[0], NULL) == CL_SUCCESS)
                cout << "    vendor: " << param.c_str() << endl;
            if (clGetDeviceInfo(deviceIDs[j], CL_DEVICE_VERSION, param.size(), &param[0], NULL) == CL_SUCCESS)
                cout << "    version: " << param.c_str() << endl;
            if (clGetDeviceInfo(deviceIDs[j], CL_DRIVER_VERSION, param.size(), &param[0], NULL) == CL_SUCCESS)
                cout << "    driver version: " << param.c_str() << endl;
        }
    }

    return EXIT_SUCCESS;
}
