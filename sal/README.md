# System Abstraction Layer (SAL)

System abstraction layer; rtl, osl and sal

`rtl`:
Platform independent strings

`osl`:
platform specific stuff, threads, dynamic loading, process, ipc, etc

Exports only C API and some inline-methods (only C++ API).
