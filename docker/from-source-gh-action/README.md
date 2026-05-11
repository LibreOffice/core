This directory contains relevant files for building a docker image from source code using GitHub Actions. The Dockerfile is used to build the image is different from the regular from-source approach as it isolates the build within docker so that the build environment is not dependent on the host environment.

Right now the engine is not built from source but uses the prebuilt binaries.

## Build locally

To build the image locally, run the following command:

```bash
docker build -t code .
```

