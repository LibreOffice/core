plugins {
    alias(libs.plugins.android.application)
}

// buildhost settings - paths and the like
apply(from = "liboSettings.gradle.kts")

val liboInstdir: String by extra
val liboUREJavaFolder: String by extra
val liboJniLibsdir: String by extra
val liboEtcFolder: String by extra
val liboUreMiscFolder: String by extra
val liboSharedResFolder: String by extra
val liboSrcRoot: String by extra
val liboWorkdir: String by extra
val liboVersionMajor: String by extra
val liboVersionMinor: String by extra
val liboGitFullCommit: String by extra
val androidNdkPath: String by extra
val androidNdkVersion: String by extra
val androidVersionName: String by extra
val buildIdShort: String by extra
val vendor: String by extra
val privacyPolicyUrl: String by extra
val androidApplicationId: String by extra
val androidMinSdkVersion: Int by extra

// compile-time dependencies
dependencies {
    implementation(fileTree(mapOf(
            "dir" to "$liboInstdir/$liboUREJavaFolder",
            "include" to listOf(
                    "java_uno.jar",
                    "libreoffice.jar",
                    "unoloader.jar"
            )
    )))
    implementation(libs.material)
    implementation(libs.constraintlayout)
    implementation(libs.preference)
}

base {
    archivesName.set("LibreOfficeViewer")
}

android {
    namespace = "org.libreoffice"
    compileSdk = 35

    ndkPath = androidNdkPath
    ndkVersion = androidNdkVersion

    defaultConfig {
        applicationId = androidApplicationId
        minSdk = androidMinSdkVersion
        versionCode = if (project.hasProperty("cmdVersionCode")) project.property("cmdVersionCode").toString().toInt() else 1
        versionName = androidVersionName
        buildConfigField("String", "BUILD_ID_SHORT", buildIdShort)
        buildConfigField("String", "VENDOR", vendor)
        buildConfigField("String", "PRIVACY_POLICY_URL", privacyPolicyUrl)
    }

    buildFeatures {
        buildConfig = true
    }

    // uses non-conventional source layout, so need to reconfigure accordingly
    // ToDo move to conventional layout, so stuff can be stripped down.
    sourceSets {
        getByName("main") {
            manifest.srcFile("AndroidManifest.xml")
            assets.srcDirs("assets")
            res.srcDirs("res", "res_generated")
            java.srcDirs("../Bootstrap/src", "src/java")
            jniLibs.srcDirs("$liboJniLibsdir")
            assets.srcDirs("assets_strippedUI")
        }

        create("quest") {
            manifest.srcFile("AndroidManifest_quest.xml")
        }
    }

    defaultConfig {
        // minSdkVersion is set in liboSettings.gradle.kts
        targetSdk = 35
        vectorDrawables.useSupportLibrary = true
    }

    buildTypes {
        debug {
            // make android studio happy...
            isJniDebuggable = true
            // would work just fine with external, but setting emulator up is a little more work
            manifestPlaceholders["installLocation"] = "auto"
        }
        release {
            manifestPlaceholders["installLocation"] = "preferExternal"
        }
    }

    flavorDimensions.add("default")
    productFlavors {
        create("strippedUI") {
            dimension = "default"
            buildConfigField("boolean", "ALLOW_EDITING", "false")
        }
        create("strippedUIEditing") {
            dimension = "default"
            buildConfigField("boolean", "ALLOW_EDITING", "true")
        }

        // product flavor for Meta Quest devices (Horizon OS)
        create("quest") {
            dimension = "default"
            buildConfigField("boolean", "ALLOW_EDITING", "false")
        }
    }

    lint {
        warningsAsErrors = true
        // ignore missing or extra translations, since these are tracked/managed via Weblate
        disable.addAll(listOf("ExtraTranslation", "MissingTranslation"))
        // don't error-out on external updates (new gradle plugin, library versions
        // or target API become available)
        // don't error-out on TypographyEllipsis, since this can be introduced with
        // new translations, and those are handled in Weblate
        informational.addAll(listOf("AndroidGradlePluginVersion", "GradleDependency", "NewerVersionAvailable", "OldTargetApi", "TypographyEllipsis"))
        // don't fail on pre-existing issues
        // These should be dealt with at some point, though.
        // To update lint-baseline.xml, just remove the file and run the build again.
        baseline = file("lint-baseline.xml")
        // s.a. lint.xml that contains further config
    }
}

// show warnings about use of deprecated API
tasks.withType<JavaCompile> {
    options.isDeprecation = true
}

/* remark inherited from makefile:
Then "assets". Let the directory structure under assets mimic
that under solver for now.

Please note that I have no idea what all of this is really necessary and for
much of this stuff being copied, no idea whether it makes any sense at all.
Much of this is copy-pasted from android/qa/sc/Makefile (where a couple of
unit tests for sc are built, and those do seem to mostly work) and
android/qa/desktop/Makefile (mmeeks's desktop demo, also works to some extent)
 */

// Assets that are unpacked at run-time into the app's data directory. These
// are files read by non-LO code, fontconfig and freetype for now, that doesn't
// understand "/assets" paths.
tasks.register<Copy>("copyUnpackAssets") {
    description = "copies assets that need to be extracted on the device"
    into("assets/unpack")

    into("program") {
        from("$liboInstdir/$liboEtcFolder/types") {
            include(
                    "offapi.rdb",
                    "oovbaapi.rdb"
            )
        }
        from("$liboInstdir/$liboUreMiscFolder") {
            include("types.rdb")
            rename("types.rdb", "udkapi.rdb")
        }
    }

    into("user/fonts") {
        from("$liboInstdir/share/fonts/truetype")
        // Note: restrict list of fonts due to size considerations - no technical reason anymore
        // ToDo: fonts would be good candidate for using Expansion Files instead
        include(
                "Liberation*.ttf",
                "Caladea-*.ttf",
                "Carlito-*.ttf",
                "Gen*.ttf",
                "opens___.ttf"
        )
    }

    into("etc/fonts") {
        from(".")
        include("fonts.conf")
        filter { line ->
            line.replace(
                    "@@APPLICATION_ID@@",
                    android.defaultConfig.applicationId.toString()
            )
        }
    }
}

tasks.register<Copy>("copyAssets") {
    description = "copies assets that can be accessed within the installed apk"
    into("assets")

    // include icons, Impress styles and required .ui files
    into("share") {
        into("config") {
            from("$liboInstdir/share/config")
            include(
                    "images_**.zip",
                    "**/simpress/**.xml",
                    "**/annotation.ui",
                    "**/hfmenubutton.ui",
                    "**/inforeadonlydialog.ui",
                    "**/pbmenubutton.ui",
                    "**/scrollbars.ui",
                    "**/tabbuttons.ui",
                    "**/tabbuttonsmirrored.ui",
                    "**/tabviewbar.ui"
            )
        }
    }

    into("program") {
        from("$liboInstdir/program")
        include("services.rdb", "services/services.rdb")

        into("resource") {
            from("$liboInstdir/$liboSharedResFolder")
            include("*en-US.res")
        }
    }
    into("share") {
        from("$liboInstdir/share") {
            // Filter data is needed by e.g. the drawingML preset shape import.
            include("registry/**", "filter/**")
            // those two get processed by mobile-config.py
            exclude("registry/main.xcd", "registry/res/registry_en-US.xcd")
        }
        // separate data files for Chinese and Japanese
        from("$liboWorkdir/CustomTarget/i18npool/breakiterator/") {
            include("*.data")
        }
    }
}

tasks.register<Copy>("copyAppResources") {
    description = "copies documents to make them available as app resources"
    into("res_generated/raw")
    from("$liboInstdir") {
        include("LICENSE", "NOTICE")
        rename("LICENSE", "license.txt")
        rename("NOTICE", "notice.txt")
    }
}

tasks.register("createStrippedConfig") {
    val preserveDir = file("assets_strippedUI/share/config/soffice.cfg/empty")
    outputs.dir("assets_strippedUI")
    outputs.dir("assets_strippedUI/share/registry/res")
    outputs.file(preserveDir)

    doLast {
        file("assets_strippedUI/share/registry/res").mkdirs()
        file("assets_strippedUI/share/config/soffice.cfg").mkdirs()
        // just empty file
        preserveDir.writeText("")
    }
}

tasks.register<Exec>("createStrippedConfigMain") {
    dependsOn("createStrippedConfig")
    inputs.files("$liboInstdir/share/registry/main.xcd", "$liboSrcRoot/android/mobile-config.py")
    outputs.file("assets_strippedUI/share/registry/main.xcd")
    executable = "$liboSrcRoot/android/mobile-config.py"
    args("$liboInstdir/share/registry/main.xcd", "assets_strippedUI/share/registry/main.xcd")
}

tasks.register<Exec>("createStrippedConfigRegistry") {
    dependsOn("createStrippedConfig")
    inputs.files("$liboInstdir/share/registry/res/registry_en-US.xcd", "$liboSrcRoot/android/mobile-config.py")
    outputs.file("assets_strippedUI/share/registry/res/registry_en-US.xcd")
    executable = "$liboSrcRoot/android/mobile-config.py"
    args("$liboInstdir/share/registry/res/registry_en-US.xcd", "assets_strippedUI/share/registry/res/registry_en-US.xcd")
    doFirst {
        file("assets_strippedUI/share/registry/res").mkdirs()
    }
}

tasks.register("createRCfiles") {
    dependsOn("copyUnpackAssets", "copyAssets")
    val sofficerc = file("assets/unpack/program/sofficerc")
    val fundamentalrc = file("assets/program/fundamentalrc")
    val bootstraprc = file("assets/program/bootstraprc")
    val unorc = file("assets/program/unorc")
    val versionrc = file("assets/program/versionrc")

    outputs.files(sofficerc, fundamentalrc, unorc, bootstraprc, versionrc)

    doLast {
        sofficerc.writeText("""
            [Bootstrap]
            Logo=1
            NativeProgress=1
            URE_BOOTSTRAP=file:///assets/program/fundamentalrc
            HOME=${"$"}APP_DATA_DIR/cache
            OSL_SOCKET_PATH=${"$"}APP_DATA_DIR/cache
            """.trimIndent())

        fundamentalrc.writeText("""
            [Bootstrap]
            LO_LIB_DIR=file://${"$"}APP_DATA_DIR/lib/
            BRAND_BASE_DIR=file:///assets
            BRAND_SHARE_SUBDIR=share
            CONFIGURATION_LAYERS=xcsxcu:${"$"}{BRAND_BASE_DIR}/share/registry res:${"$"}{BRAND_BASE_DIR}/share/registry
            URE_BIN_DIR=file:///assets/ure/bin/dir/nothing-here/we-can/exec-anyway
            """.trimIndent())

        bootstraprc.writeText("""
            [Bootstrap]
            InstallMode=<installmode>
            ProductKey=LibreOffice ${liboVersionMajor}.${liboVersionMinor}
            UserInstallation=file://${"$"}APP_DATA_DIR
            """.trimIndent())

        unorc.writeText("""
            [Bootstrap]
            URE_INTERNAL_LIB_DIR=file://${"$"}APP_DATA_DIR/lib/
            UNO_TYPES=file://${"$"}APP_DATA_DIR/program/udkapi.rdb file://${"$"}APP_DATA_DIR/program/offapi.rdb file://${"$"}APP_DATA_DIR/program/oovbaapi.rdb
            UNO_SERVICES=file:///assets/program/services.rdb file:///assets/program/services/services.rdb
            """.trimIndent())

        versionrc.writeText("""
            [Version]
            AllLanguages=en-US
            buildid=${liboGitFullCommit}
            ReferenceOOoMajorMinor=4.1
            """.trimIndent())
    }
}

// creating the UI stuff is cheap, don't bother only applying it for the flavor...
tasks.preBuild {
    dependsOn("createRCfiles", "createStrippedConfigMain", "createStrippedConfigRegistry", "copyAppResources")
}

tasks.clean {
    dependsOn("cleanCopyAssets", "cleanCreateStrippedConfig")
}
