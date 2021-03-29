# Registry Reading, etc

This provides tools for dealing with the legacy binary types database
format, still in use by extensions and the core code. While the actual
binary file format is implemented by the `store` code, the wrapper
that turns this into a type registry is implemented here.

While this code is primarily used in only two modes:

* linear write / concatenate
* random access read

The API unfortunately exposes a random-access write approach, which -
while ~unused needs before we can re-write this away from the store
backend.
