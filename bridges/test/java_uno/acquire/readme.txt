Test scenarios:

> cd .../bridges/<outdir>/bin
> ./testacquire-java-server &
> sleep 3 # wait for server to accept connection
> ./testacquire-java-client

> cd .../bridges/<outdir>/bin
> ./testacquire-java-server &
> sleep 3 # wait for server to accept connection
> ./testacquire-native-client

> cd .../bridges/<outdir>/bin
> ./testacquire-native-server &
> sleep 3 # wait for server to accept connection
> ./testacquire-java-client

> cd .../bridges/<outdir>/bin
> ./testacquire-native-server &
> sleep 3 # wait for server to accept connection
> ./testacquire-native-client
