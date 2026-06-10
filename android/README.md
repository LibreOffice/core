<!-- build-doc-start -->
# Build the Android app

These instructions build the Collabora Office app for Android. The native parts must be built on Linux; you build the engine first, then the app, from the same monorepo.

## Requirements

* **A computer running Linux.** The native parts of the Android app cannot currently be built on Windows.
* **The Android platform tools, including a compatible NDK.** Builds have been tested with Android NDK 23.0.7599858; other NDK versions may or may not work.
* **An Android or ChromeOS device, or a simulator.** The app can run in a simulator, though some bugs may appear in a simulator but not on a physical device or vice versa — if you can, build and run on a physical device. [Connect the device or simulator to your computer using adb](https://developer.android.com/tools/adb#Enabling).

## Clone the monorepo

All the source code now lives in a single Gerrit monorepo; the former Collabora Office core is the `engine/` subdirectory of the `online` repo, so there is no separate repository to clone any more. Code review happens on [Gerrit](https://gerrit.collaboraoffice.com/), not GitHub pull requests; see the [first contribution guide](https://forum.collaboraonline.com/t/your-first-pull-request/41) for the full workflow.

Collabora Online is hosted on Gerrit as a single monorepo: all the source code lives in one repository, with the former Collabora Office core under `engine/`.

For an anonymous read-only clone (no account needed):
```bash
git clone https://gerrit.collaboraoffice.com/online collabora-office
```

If you have a Gerrit account and plan to push changes for review, clone over SSH instead:
```bash
git clone ssh://YOUR_USERNAME@gerrit.collaboraoffice.com:29418/online collabora-office
```

See the [first contribution guide](https://forum.collaboraonline.com/t/your-first-pull-request/41) for the full Gerrit workflow (SSH key, `commit-msg` hook, pushing to `refs/for/main`).

Switch to the local clone's directory:
```bash
cd collabora-office
```

## Build the engine

The engine lives under `engine/` inside the monorepo - that is where you do the engine build:

The former Collabora Office core lives inside the `online` monorepo under `engine/`, so a separate clone is no longer needed. If you have not cloned the monorepo yet, run the `clone-online` step above first. Then move into the engine tree:

```bash
cd engine
git checkout main
```

For a localized (translated) user interface, also clone the translations repository into `engine/translations` (you are now inside `engine/`); the engine's `--with-lang` picks up the `.po` files from there:

```bash
git clone https://gerrit.collaboraoffice.com/translations translations
```

If you already have the monorepo cloned for another job, you may [use git worktrees to speed up this step](https://git-scm.com/docs/git-worktree).

### Configure

Decide what architecture you are going to build for. This will depend on your
android device's ABI. We support building for armeabi-v7a, arm64-v8a, x86 and
x86_64.

If you're not sure what your phone's architecture is, you can either research
online or use adb to get a list of valid architectures

    $ adb shell getprop ro.product.cpu.abilist
    arm64-v8a,armeabi-v7a,armeabi

Create a file called `autogen.input` in the `engine/` directory with the
following content:

    --build=x86_64-unknown-linux-gnu
    --with-android-ndk=/home/$USER/Android/Sdk/ndk/23.0.7599858
    --with-android-sdk=/home/$USER/Android/Sdk
    --enable-sal-log
    --enable-dbgutil

You also need to add a line specifying which architecture you're building for

#### For arm64-v8a

    --with-distro=CPAndroidAarch64

#### For armeabi-v7a

    --with-distro=CPAndroid

#### For x86_64

    --with-distro=CPAndroidX86_64

#### For x86

    --with-distro=CPAndroidX86

---

For example, if you have a device that supports `arm64-v8a` your autogen.input
should contain this content

    --build=x86_64-unknown-linux-gnu
    --with-android-ndk=/home/$USER/Android/Sdk/ndk-bundle
    --with-android-sdk=/home/$USER/Android/Sdk
    --enable-sal-log
    --with-distro=CPAndroidAarch64
    --enable-dbgutil

Finally, run

    ./autogen.sh

### Build

Run `make` and wait a while for the build to finish...

    make

## Build Collabora Office

POCO and libzstd are built as part of the engine, one
copy per ABI, and taken from its workdir, so they no longer need to be built
separately for Android.

### Configure

Let's set some variables based on what we just built...

    export ABI=arm64-v8a
    export CO_BUILDDIR=/opt/libreoffice

...remember to change your ABI to the ABI you're building the app for, and
CO_BUILDDIR to the `engine/` subdirectory of the monorepo where you built
the engine.

Now step back to the top of the monorepo (one level up from `engine/`) and configure the Collabora Office build

    cd ..
    ./autogen.sh
    ./configure --enable-androidapp \
                --with-lo-builddir=${CO_BUILDDIR} \
                --enable-debug \
                --with-android-abi=${ABI}

### Build

Once again, after configuring the build you can run it with `make`

    make

## Build the app

### Option 1: Using Android studio

This is the recommended way to build the Android app

- Open Android studio
- Open the `android` subdirectory as a project
- Use `build -> make project` to run the build

### Option 2: Using gradle from the command line

It may be harder to debug the app without using Android Studio. Unless you have
prior experience with debugging Android apps outside Android Studio (or
otherwise aren't able to use Android Studio) we recommend you follow Option 1

    cd android
    ./gradlew build

## Debugging

To debug the native engine code in Android Studio, you need the debugging
symbols and to setup Android Studio to actually read & use them.

### Add android/obj/local/armeabi-v7a from the engine as a Symbol Directory

In Android Studio, choose Run -> Debug... -> Edit Configurations...

There go to the Android App -> app, choose the Debugger tab, and:

    Debug type: Auto (or Dual)

Symbol Directories: here add the full path, like...

    /local/libreoffice/master-android/android/obj/local/${ABI}

...making sure to substitute `${ABI}` for the ABI you builts for

This path contains the non-stripped version of the liblo-native-code.so, and
the debugger will read the symbols from that one (even if the APK contains
the stripped version). *NB* ensure that this is before any internal source
directories - since the internal source contains stripped native code.

Alternatively you can add the following to your ~/.lldbinit instead:

    settings set target.inline-breakpoint-strategy always
    settings append target.exec-search-paths /local/libreoffice/master-android/android/obj/local/${ABI}

To use pretty printers for types like OUString, add the following to your
~/.lldbinit:

    command script import '/local/libreoffice/master-android/solenv/lldb/libreoffice/LO.py'

From now on, you will be able to debug directly in the Android Studio
debugger.  Happy debugging!

## Cross-compiling with icecream to speed up your build

If you use icecream for parallel building, you can use it for
cross-compilation too.

    # first generate a tarball with the toolchain (once)
    icecc-create-env ~/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi21-clang ~/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi21-clang++

And add it and the paths to the compiler as the first things to the
autogen.input:

    CC=icecc [here copy what the output of ./autogen.sh without icecream said for C compiler]
    CXX=icecc [here copy what the output of ./autogen.sh without icecream said for C++ compiler]
    ICECC_VERSION=/path/to/the/tarball/generated/above/955ceb546ceb7a5715bf0223ddd788fe.tar.gz
    --with-parallelism=[amount of cpu threads in your icecream farm]
    --enable-icecream
    [...the original autogen.input...]

So the result will look something like this:

    CC=icecc /home/$USER/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/bin/clang -mthumb -march=armv7-a -mfloat-abi=softfp -mfpu=neon -Wl,--fix-cortex-a8 -gcc-toolchain /home/$USER/Android/Sdk/ndk-bundle/to>
    CXX=icecc /home/$USER/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++ -mthumb -march=armv7-a -mfloat-abi=softfp -mfpu=neon -Wl,--fix-cortex-a8 -gcc-toolchain /home/$USER/Android/Sdk/ndk-bundle>
    ICECC_VERSION=/local/libreoffice/android/955ceb546ceb7a5715bf0223ddd788fe.tar.gz
    --with-parallelism=25
    --enable-icecream
    --build=x86_64-unknown-linux-gnu
    --with-android-ndk=/home/$USER/Android/Sdk/ndk-bundle
    --with-android-sdk=/home/$USER/Android/Sdk
    --with-distro=CPAndroid
    --enable-sal-log
<!-- build-doc-end -->
