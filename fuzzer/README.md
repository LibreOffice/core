# COOL fuzzers

These fuzzers are meant to be built and executed inside a sanitizers environment (source the
toplevel `autogen.env-san`).

online.git can be built the usual way, just the additional `--enable-fuzzers`
flag is needed to build the fuzzers. It is useful to do this in a separate
build tree, since the fuzzers config doesn't produce a `coolwsd` binary.

## Building with libFuzzer (default)

```
./configure --enable-fuzzers [other options]
make
```

Run the fuzzers like this:

- Admin:

```
fuzzer/admin_fuzzer -max_len=16384 fuzzer/admin-data/
```

- ClientSession:

```
fuzzer/clientsession_fuzzer -max_len=16384 fuzzer/data/
```

- HttpResponse:

```
fuzzer/httpresponse_fuzzer -max_len=16384 fuzzer/httpresponse-data/
```

- Collab:

```
fuzzer/collab_fuzzer -max_len=16384 fuzzer/collab-data/
```

- HttpEcho:

```
fuzzer/httpecho_fuzzer -max_len=16384 fuzzer/httpecho-data/
OR, better:
./fuzzer/httpecho-fuzz.sh
```

## Building with AFL++

```
./configure --enable-fuzzers=afl --with-sanitizer=address,undefined [other options]
make
```

The configure script auto-detects the AFL++ compiler (afl-clang-lto++,
afl-clang-fast++, or afl-c++). The existing LLVMFuzzerTestOneInput
harnesses work unchanged under AFL++ persistent mode.

Run individual targets:

```
fuzzer/run-afl.sh admin
fuzzer/run-afl.sh clientsession
fuzzer/run-afl.sh clientrequest
fuzzer/run-afl.sh httpresponse
fuzzer/run-afl.sh httpecho
```

Extra afl-fuzz flags can be appended, e.g.:

```
fuzzer/run-afl.sh admin -t 2000
```

For parallel fuzzing, just run the script in multiple terminals.
It auto-selects the right instance type (main, san, cmplog, cmpcov,
then native secondaries with varied settings):

```
fuzzer/run-afl.sh admin   # 1st: -M main (native)
fuzzer/run-afl.sh admin   # 2nd: -S san01 (native + -w san)
fuzzer/run-afl.sh admin   # 3rd: -S cmplog01 (native + -c cmplog)
fuzzer/run-afl.sh admin   # ... and so on
```
