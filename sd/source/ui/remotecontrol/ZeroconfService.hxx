#ifndef ZEROCONF_SERVICE
#define ZEROCONF_SERVICE

#include <string>

class ZeroconfService
{
private:
    string name;
    uint port;

public:
    ZeroconfService(string aname = "", uint aport = 1599);
    ~ZeroconfService();
    // Clean up the service when closing
    virtual void clear() = 0;
    // Bonjour for OSX, Avahi for Linux
    virtual void setup() = 0;
};

#endif
